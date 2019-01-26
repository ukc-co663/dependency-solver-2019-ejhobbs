#include "repo_package.h"

package* package_fromJson(const cJSON* jsonPkg){
  cJSON* name = cJSON_GetObjectItemCaseSensitive(jsonPkg, "name");
  cJSON* size = cJSON_GetObjectItemCaseSensitive(jsonPkg, "size");
  cJSON* version = cJSON_GetObjectItemCaseSensitive(jsonPkg, "version");
  cJSON* depends = cJSON_GetObjectItemCaseSensitive(jsonPkg, "depends");
  cJSON* conflicts = cJSON_GetObjectItemCaseSensitive(jsonPkg, "conflicts");

  /* Check everything's in order */
  if (!cJSON_IsNumber(size) || !cJSON_IsString(name) ||
      !cJSON_IsString(version)) {
    fprintf(stderr, "Malformed package information\n");
    return NULL;
  }
  package* parsed = malloc(sizeof *parsed);
  parsed->name = name->valuestring;
  parsed->size = size->valueint;
  parsed->version = versionFromString(version->valuestring);

  int numDepends = 0;
  int numConflicts = 0;

  if (depends != NULL && cJSON_IsArray(depends)) {
    //Depends is a list of lists for no apparent reason
    getAllDependencies(parsed, depends);
  }
  if (conflicts != NULL && cJSON_IsArray(conflicts)) {
    numConflicts = cJSON_GetArraySize(conflicts);
    parsed->conflicts = getAllRelations(conflicts, numConflicts);
  }

  parsed->cDepends = numDepends;
  parsed->cConflicts = numConflicts;

  return parsed;
}

void getAllDependencies(package* pkg, const cJSON* deps) {
  int numGroups = cJSON_GetArraySize(deps);
  relation** groups = calloc(numGroups, sizeof(relation*));
  cJSON* thisItem = NULL;
  int curGroup = 0;
  cJSON_ArrayForEach(thisItem, deps) {
    int thisSize = cJSON_GetArraySize(thisItem);
    relation* theseRelations = getAllRelations(thisItem, thisSize);

    groups[curGroup] = theseRelations;
    curGroup++;
  }
  pkg->cDepends = numGroups;
  pkg->depends = groups;
}

relation* getAllRelations(const cJSON* relationList, int count) {
  relation* relations = calloc(count, sizeof(relation));
  cJSON* rel = NULL;
  int i = 0;
  cJSON_ArrayForEach(rel, relationList) {
    relations[i] = parseRelation(rel->valuestring);
    i++;
  }
  return relations;
}

relation parseRelation(char* rel) {
  int sName = 0; /* Size of pkg name */
  int comp = 0; /* Start with any */
  char* curChar = rel;
  /* Loop over the string until we get to the first number, ie the version starts */
  while(curChar < rel + strlen(rel) && (*curChar > 0x39 || *curChar < 0x30)){
    switch(*curChar) {
      case '<':
        comp |= _lt;
        break;
      case '>':
        comp |= _gt;
        break;
      case '=':
        comp |= _eq;
        break;
      default:
        sName++;
        break;
    }
    curChar++;
  }
  char* name = calloc(sName+1, sizeof(char));
  strncpy(name, rel, sName);
  name[sName] = 0;

  relation newRel = {name, versionFromString(curChar), comp};
  return newRel;
}


version versionFromString(char* versionString) {
  int* parts = NULL;
  int vSize = 0;
  char delim[] = ".";
  char* part = strtok(versionString, delim);
  while (part != NULL) {
    int* tmp = realloc(parts, (vSize+1)*sizeof(int));
    if(tmp != NULL) {
      parts = tmp;
      parts[vSize] = atoi(part);
      part = strtok(NULL, delim);
      vSize++;
    } else {
      perror("Something went wrong");
    }
  }
  version vers = {vSize, parts};
  return vers;
}

void relation_free(int s, relation* r) {
  for (int i = 0; i < s; i++) {
    free(r[i].name);
    version_free(&(r[i].version));
  }
  free(r);
}

void version_free(version* v) {
  if (v->size > 0) {
    free(v->val);
  }
}

void package_free(package* p) {
  if (p->cDepends > 0 && p->depends != NULL) {
    for(int i = 0; i < p->cDepends; i++) {
      relation_free(p->cDepends, p->depends[i]);
    }
    free(p->depends);
  }
  if (p->cConflicts > 0 && p->conflicts != NULL) relation_free(p->cConflicts, p->conflicts);
  version_free(&(p->version));
  free(p);
}

void package_prettyPrint(const package* p) {
  printf("\n--[ %s ]--\n", p->name);
  printf("Size: %d\n", p->size);
  printf("Version: ");
  version_prettyPrint(&(p->version));
  printf("\n");
  if(p->cDepends > 0 && p->depends != NULL) {
    printf("Depends:\n");
    for(int i = 0; i < p->cDepends; i++) {
      relations_prettyPrint(p->cDepends, (const relation*) p->depends[i]);
    }
  }
  if(p->cConflicts > 0 && p->conflicts != NULL) {
    printf("Conflicts:\n");
    relations_prettyPrint(p->cConflicts, (const relation*) p->conflicts);
  }
}

void version_prettyPrint(const version* v){
  if (v->size > 0 && v->val != NULL) {
    printf("%d", v->val[0]);
    for (int i=1; i < v->size; i++) {
      printf(".%d", v->val[i]);
    }
  }
}

void relations_prettyPrint(int s, const relation* rs) {
  for (int i=0; i < s; i++) {
    printf("\t %s", rs[i].name);
    comp_prettyPrint(&(rs[i].comp));
    version_prettyPrint(&(rs[i].version));
    printf("\n");
  }
}

void comp_prettyPrint(const int* comp){
  if(*comp & _gt) printf(">");
  if(*comp & _lt) printf("<");
  if(*comp & _eq) printf("=");
}

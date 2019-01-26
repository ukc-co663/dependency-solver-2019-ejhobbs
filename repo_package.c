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

  int numConflicts = 0;

  if (depends != NULL && cJSON_IsArray(depends)) {
    getAllDependencies(parsed, depends);
  }
  if (conflicts != NULL && cJSON_IsArray(conflicts)) {
    numConflicts = cJSON_GetArraySize(conflicts);
    parsed->conflicts = getAllRelations(conflicts, numConflicts);
    parsed->cConflicts = numConflicts;
  }

  return parsed;
}

void getAllDependencies(package* pkg, const cJSON* deps) {
  int numGroups = cJSON_GetArraySize(deps);
  relation_group* groups = calloc((size_t) numGroups, sizeof(relation_group));
  cJSON* thisItem = NULL;
  int curGroup = 0;
  cJSON_ArrayForEach(thisItem, deps) {
    int thisSize = cJSON_GetArraySize(thisItem);
    relation* theseRelations = getAllRelations(thisItem, thisSize);

    relation_group grp = {thisSize, theseRelations};
    groups[curGroup] = grp;
    curGroup++;
  }
  pkg->cDepends = numGroups;
  pkg->depends = groups;
}

relation* getAllRelations(const cJSON* relationList, int count) {
  relation* relations = calloc((size_t) count, sizeof(relation));
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
  int sym = 0;
  char* curChar = rel;
  /* Loop over the string until we get to the first symbol, ie end of the name*/
  while(sName < strlen(rel) && !sym){
    switch(*curChar) {
      case '<':
      case '>':
      case '=':
        sym = 1;
        break;
      default:
        sName++;
        break;
    }
    if(!sym){
        curChar++;
    }
  }
  char* name = calloc((size_t) sName+1, sizeof(char));
  strncpy(name, rel, sName);
  name[sName] = 0;
  int num = 0;
  while(curChar < rel + strlen(rel) && !num) {
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
              num=1;
              break;
      }
      if (!num) {
          curChar++;
      }
  }
  version ver = {0, NULL};
  if(num) {
      ver = versionFromString(curChar);
  }

  relation newRel = {name, ver, comp};
  return newRel;
}


version versionFromString(char* versionString) {
  int* parts = NULL;
  int vSize = 0;
  char delim[] = ".";
  char* part = strtok(versionString, delim);
  if(part == NULL) {
      parts = malloc(sizeof(int));
      vSize = 1;
      *parts = atoi(versionString);
  } else {
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
      relation_free(p->depends[i].size, p->depends[i].relations);
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
      relations_prettyPrint(p->depends[i].size, (const relation*) p->depends[i].relations);
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

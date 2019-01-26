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

  if (depends != NULL && cJSON_IsArray(depends)) {
    /* Depends is a list of lists for no apparent reason */
    int numDepends = 0;
    relation** relations = NULL;
    cJSON* thisItem = NULL;
    cJSON_ArrayForEach(thisItem, depends) {
      int thisSize = cJSON_GetArraySize(thisItem);
      relation** theseRelations = getAllRelations(thisItem, thisSize);

      relations = realloc(relations, (numDepends+thisSize)*sizeof(relation*));
      memcpy(relations + numDepends, theseRelations, thisSize*sizeof(relation*));

      numDepends += thisSize;
    }
    parsed->cDepends = numDepends;
    parsed->depends = relations;
  }
  if (conflicts != NULL && cJSON_IsArray(conflicts)) {
    int numConflicts = cJSON_GetArraySize(conflicts);
    parsed->cConflicts = numConflicts;
    parsed->conflicts = getAllRelations(conflicts, numConflicts);
  }

  return parsed;
}

relation** getAllRelations(const cJSON* relationList, int count) {
  relation** relations = calloc(count, sizeof(relation*));
  relation** thisRelation = relations;
  const cJSON* rel = NULL;
  cJSON_ArrayForEach(rel, relationList) {
    *thisRelation = parseRelation(rel->valuestring);
    thisRelation++;
  }
  return relations;
}

relation* parseRelation(char* rel) {
  relation* newRel = calloc(1, sizeof(relation));
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

  newRel->name = name;
  newRel->version = versionFromString(curChar);
  newRel->comp = comp;
  return newRel;
}


version* versionFromString(char* versionString) {
  version* vers = calloc(1, sizeof(version));
  int* v = NULL;
  int vSize = 0;
  char delim[] = ".";
  char* part = strtok(versionString, delim);
  while (part != NULL) {
    v = realloc(v, vSize++);
    v[vSize-1] = atoi(part);
    part = strtok(NULL, delim);
  }
  vers->parts = vSize;
  vers->val = v;
  return vers;
}

void relation_free(int s, relation** r) {
  for (int i = 0; i < s; i++) {
    free(r[i]->name);
    free(r[i]->version->val);
    free(r[i]->version);
    free(r[i]);
  }
  free(r);
}

void package_free(package* p) {
  relation_free(p->cDepends, p->depends);
  relation_free(p->cConflicts, p->conflicts);
  free(p->version->val);
  free(p->version);
  free(p);
}

void package_prettyPrint(const package* p) {
  printf("\n--[ %s ]--\n", p->name);
  printf("Size: %d\n", p->size);
  printf("Version: ");
  version_prettyPrint(p->version);
  printf("\n");
  if(p->depends != NULL) {
    printf("Depends:\n");
    relations_prettyPrint(p->cDepends, (const relation**) p->depends);
  }
  if(p->conflicts != NULL) {
    printf("Conflicts:\n");
    relations_prettyPrint(p->cConflicts, (const relation**) p->conflicts);
  }
}

void version_prettyPrint(const version* v){
  if (v->parts > 0) {
    printf("%d", v->val[0]);
    for (int i=1; i < v->parts; i++) {
      printf(".%d", v->val[i]);
    }
  }
}

void relations_prettyPrint(int s, const relation** rs) {
  for (int i=0; i < s; i++) {
    printf("\t %s", rs[i]->name);
    comp_prettyPrint(&(rs[i]->comp));
    version_prettyPrint(rs[i]->version);
    printf("\n");
  }
}

void comp_prettyPrint(const int* comp){
  if(*comp & _gt) printf(">");
  if(*comp & _lt) printf("<");
  if(*comp & _eq) printf("=");
}

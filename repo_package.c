#include "repo_package.h"

package* package_fromJson(const cJSON* jsonPkg){
  cJSON* name = cJSON_GetObjectItemCaseSensitive(jsonPkg, "name");
  cJSON* size = cJSON_GetObjectItemCaseSensitive(jsonPkg, "size");
  cJSON* version = cJSON_GetObjectItemCaseSensitive(jsonPkg, "version");
  cJSON* depends = cJSON_GetObjectItemCaseSensitive(jsonPkg, "depends");
  cJSON* conflicts = cJSON_GetObjectItemCaseSensitive(jsonPkg, "conflicts");

  /* Check everything's in order */
  if (!cJSON_IsNumber(size) || !cJSON_IsString(name) ||
      !cJSON_IsString(version) || !cJSON_IsArray(depends) ||
      !cJSON_IsArray(conflicts)) {
    fprintf(stderr, "Malformed package information\n");
    return NULL;
  }

  int numDepends = cJSON_GetArraySize(depends);
  int numConflicts = cJSON_GetArraySize(conflicts);

  package* parsed = malloc(sizeof *parsed);
  parsed->name = name->valuestring;
  parsed->size = size->valueint;
  parsed->version = versionFromString(version->valuestring);
  parsed->cDepends = numDepends;
  parsed->depends = getAllRelations(depends, numDepends);
  parsed->cConflicts = numConflicts;
  parsed->conflicts = getAllRelations(conflicts, numConflicts);
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


int* versionFromString(char* versionString) {
  int* v = calloc(_numversions, sizeof(int));
  char delim[] = ".";
  char* part = strtok(versionString, delim);
  int* thisVersion = v;
  while (part != NULL && thisVersion < v+_numversions ) {
    *thisVersion = atoi(part);
    thisVersion++;
    part = strtok(NULL, delim);
  }
  return v;
}

void package_free(package* p) {
  free(p->version);
  for (int i = 0; i < p->cDepends; i++) {
    free(p->depends[i]->name);
    free(p->depends[i]);
  }
  for (int i = 0; i < p->cConflicts; i++) {
    free(p->conflicts[i]->name);
    free(p->conflicts[i]);
  }
  free(p->depends);
  free(p);
}

void package_prettyPrint(const package* p) {
  printf("\n--[ %s ]--\n", p->name);
  printf("Size: %d\n", p->size);
  printf("Depends:\n");
  for (int i=0; i < p->cDepends; i++) {
    printf("\t %s ", p->depends[i]->name);
    printf(": %d.%d.%d\n"
        , p->depends[i]->version[MAJOR]
        , p->depends[i]->version[MINOR]
        , p->depends[i]->version[PATCH]);
  }
  printf("Conflicts:\n");
  for (int i=0; i < p->cConflicts; i++) {
    printf("\t %s ", p->conflicts[i]->name);
    printf(": %d.%d.%d\n"
        , p->conflicts[i]->version[MAJOR]
        , p->conflicts[i]->version[MINOR]
        , p->conflicts[i]->version[PATCH]);
  }
  printf("Version: %d.%d.%d\n", p->version[MAJOR], p->version[MINOR], p->version[PATCH]);
}

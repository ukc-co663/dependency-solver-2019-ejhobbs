#include "repo_package.h"

package* package_fromJson(const cJSON* jsonPkg){
  cJSON* name = cJSON_GetObjectItemCaseSensitive(jsonPkg, "name");
  cJSON* size = cJSON_GetObjectItemCaseSensitive(jsonPkg, "size");
  cJSON* version = cJSON_GetObjectItemCaseSensitive(jsonPkg, "version");
  cJSON* depends = cJSON_GetObjectItemCaseSensitive(jsonPkg, "depends");

  /* Check everything's in order */
  if (!cJSON_IsNumber(size) || !cJSON_IsString(name) ||
      !cJSON_IsString(version) || !cJSON_IsArray(depends)) {
    fprintf(stderr, "Malformed package information\n");
    return NULL;
  }
  int numDepends = cJSON_GetArraySize(depends);

  package* parsed = malloc(sizeof *parsed);
  parsed->name = name->valuestring;
  parsed->size = size->valueint;
  parsed->version = versionFromString(version->valuestring);
  parsed->cDepends = numDepends;
  parsed->depends = getAllDependencies(depends, numDepends);
  return parsed;
}

dependency** getAllDependencies(const cJSON* depList, int count) {
  dependency** dependencies = calloc(count, sizeof(dependency*));
  dependency** thisDep = dependencies;
  const cJSON* dep = NULL;
  cJSON_ArrayForEach(dep, depList) {
    *thisDep = getDependency(dep->valuestring);
    thisDep++;
  }
  return dependencies;
}

dependency* getDependency(char* dep) {
  dependency* newDep = calloc(1, sizeof(dependency));
  int sName = 0; /* Size of pkg name */
  int relation = 0; /* Start with any */
  char* curChar = dep;
  /* Loop over the string until we get to the first number, ie the version starts */
  while(curChar < dep + strlen(dep) && (*curChar > 0x39 || *curChar < 0x30)){
    switch(*curChar) {
      case '<':
        relation |= _lt;
        break;
      case '>':
        relation |= _gt;
        break;
      case '=':
        relation |= _eq;
        break;
      default:
        sName++;
        break;
    }
    curChar++;
  }
  char* name = calloc(sName+1, sizeof(char));
  strncpy(name, dep, sName);
  name[sName] = 0;

  newDep->name = name;
  newDep->version = versionFromString(curChar);
  newDep->comp = relation;
  return newDep;
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
  printf("Version: %d.%d.%d\n", p->version[MAJOR], p->version[MINOR], p->version[PATCH]);
}

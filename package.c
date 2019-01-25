#include "package.h"

package* package_fromJson(cJSON* jsonPkg){
  cJSON* name = cJSON_GetObjectItemCaseSensitive(jsonPkg, "name");
  cJSON* size = cJSON_GetObjectItemCaseSensitive(jsonPkg, "size");
  cJSON* version = cJSON_GetObjectItemCaseSensitive(jsonPkg, "version");
  if (!cJSON_IsNumber(size) || !cJSON_IsString(name) || !cJSON_IsString(version)) {
    fprintf(stderr, "Malformed package information\n");
    return NULL;
  }

  package* parsed = malloc(sizeof *parsed);

  parsed->name = name->valuestring;
  parsed->size = size->valueint;
  parsed->version = versionFromString(version->valuestring);
  return parsed;
}

void package_prettyPrint(package* p) {
  printf("-------------------\n");
  printf("Name: %s\n", p->name);
  printf("Size: %d\n", p->size);
  printf("Version: %d.%d.%d\n", p->version[MAJOR], p->version[MINOR], p->version[PATCH]);
  printf("-------------------\n");
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
  free(p);
}

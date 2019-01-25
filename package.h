#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cJSON.h>

#define _numversions 3
#define MAJOR 0
#define MINOR 1
#define PATCH 2

typedef struct package {
  char* name;
  int size;
  int* version;
} package;

package* package_fromJson(cJSON*);
int* versionFromString(char*);
void package_prettyPrint(package*);

void package_free(package*);

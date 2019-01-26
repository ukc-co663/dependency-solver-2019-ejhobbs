#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cJSON.h>

#define _numversions 3
#define MAJOR 0
#define MINOR 1
#define PATCH 2

/* Relation flags */
#define _gt 1
#define _lt 2
#define _eq 4


typedef struct dependency {
  char* name;
  int* version;
  int comp;
} dependency;

typedef struct package {
  char* name;
  int size;
  int* version;
  int cDepends;
  dependency** depends;
} package;

package* package_fromJson(const cJSON*);
int* versionFromString(char*);
dependency** getAllDependencies(const cJSON*, int);
dependency* getDependency(char*);
void package_prettyPrint(const package*);
void package_free(package*);

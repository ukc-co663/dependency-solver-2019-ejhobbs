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


typedef struct relation {
  char* name;
  int* version;
  int comp;
} relation;

typedef struct package {
  char* name;
  int size;
  int* version;
  int cDepends;
  relation** depends;
  int cConflicts;
  relation** conflicts;
} package;

package* package_fromJson(const cJSON*);
int* versionFromString(char*);
relation** getAllRelations(const cJSON*, int);
relation* parseRelation(char*);
void package_prettyPrint(const package*);
void package_free(package*);

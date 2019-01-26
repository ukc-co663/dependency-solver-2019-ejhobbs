#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cJSON.h>
#ifndef repo_package
#define repo_package
/* Relation flags */
#define _gt 1
#define _lt 2
#define _eq 4


typedef struct version {
  int size;
  int* val;
} version;

typedef struct relation {
  char* name;
  version version;
  int comp;
} relation;

typedef struct package {
  char* name;
  int size;
  version version;
  int cDepends;
  relation** depends;
  int cConflicts;
  relation* conflicts;
} package;

package* package_fromJson(const cJSON*);
version versionFromString(char*);
void getAllDependencies(package*, const cJSON*);
relation* getAllRelations(const cJSON*, int);
relation parseRelation(char*);
void package_prettyPrint(const package*);
void relation_free(int, relation*);
void version_free(version*);
void package_free(package*);
void version_prettyPrint(const version*);
void relations_prettyPrint(int, const relation*);
void comp_prettyPrint(const int* comp);
#endif

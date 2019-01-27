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

typedef struct relation_group {
    int size;
    relation* relations;
} relation_group;

typedef struct package {
  char* name;
  int size;
  version version;
  int cDepends;
  relation_group* depends;
  int cConflicts;
  relation* conflicts;
} package;

typedef struct package_group {
    int size;
    package** packages;
} package_group;

package_group package_getAll(const cJSON*);
int package_getIndex(package_group*);
void package_prettyPrint(const package*);
void package_freeAll(package_group);

int comparePkg(const void* p, const void* q) {
    const package* l = *(const package**)p;
    const package* r = *(const package**)q;

    return strcmp(l->name, r->name);
}
#endif

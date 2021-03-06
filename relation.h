#include "lib/cJSON/cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef SOLVE_RELATION_H
#define SOLVE_RELATION_H
/* Relation flags */
#define _gt 1
#define _lt 2
#define _eq 4

typedef struct version {
  int size;
  int *val;
} version;

typedef struct relation {
  char *name;
  struct version version;
  char comp;
} relation;

typedef struct relation_group {
  int size;
  relation *relations;
} relation_group;

int relation_satisfiedByVersion(version *, relation *);
int relation_satisfiesConstraint(int, char);
int relation_compareVersion(version *, version *);
relation *relation_getAll(const cJSON *, int);
relation parseRelation(char *);
version parseVersion(char *);
void relation_free(relation *);
void relation_freeAll(int, relation *);
void version_free(version *);
void relation_prettyPrint(relation *);
void version_prettyPrint(version *v);
#endif // SOLVE_RELATION_H

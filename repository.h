#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cJSON.h>
#include "file.h"
#ifndef repository
#define repository
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

typedef struct repo_repository {
    int size;
    package** packages;
    cJSON* json;
} repo_repository;

repo_repository repo_getFromFile(const char*);
repo_repository repo_getAll(const cJSON *);
int repo_getPackageIndex(const repo_repository *, const char *);
void repo_freeAll(repo_repository);
#endif

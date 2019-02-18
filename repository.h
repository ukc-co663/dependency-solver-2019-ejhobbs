#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lib/cJSON/cJSON.h"
#include "file.h"
#include "relation.h"
#ifndef repository
#define repository

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
int repo_getPackageIndex(const repo_repository *, relation*);
void repo_freeAll(repo_repository*);
#endif

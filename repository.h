#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lib/cJSON/cJSON.h"
#include "file.h"
#include "relation.h"
#ifndef repository_h
#define repository_h

typedef struct package {
  char* name;
  int seen; /* prevents endless loops in solver */
  int size;
  struct version version;
  int cDepends;
  relation_group* depends;
  int cConflicts;
  relation* conflicts;
} package;

typedef struct repository {
    int size;
    package** packages;
    cJSON* json;
} repository;

repository repo_getFromFile(const char*);
repository repo_getAll(const cJSON *);
int repo_getPackageIndex(const repository *, relation*);
void repo_freeAll(repository*);
#endif

#include "cJSON.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef SOLVE_RELATION_H
#define SOLVE_RELATION_H
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

relation parseRelation(char*);
version parseVersion(char*);


#endif //SOLVE_RELATION_H

#ifndef SOLVE_CONSTRAINTS_H
#define SOLVE_CONSTRAINTS_H

#include "relation.h"
#include "file.h"
#define _in '+'
#define _rm '-'

typedef struct constraint {
    char op; /* install or remove */
    relation pkg;

} constraint;

typedef struct constraints {
    int size;
    constraint* constraints;
    cJSON* json;
} constraints;

constraints constraints_getFromFile(char*);
void constraints_freeAll(constraints*);

#endif //SOLVE_CONSTRAINTS_H

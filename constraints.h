#ifndef SOLVE_CONSTRAINTS_H
#define SOLVE_CONSTRAINTS_H

#include "relation.h"
#include "file.h"

#define C_INSTALL '+'
#define C_REMOVE  '-'
typedef struct constraint {
    char op; /* install or remove */
    relation pkg;

} constraint;

/* TODO convert to LL */
typedef struct constraint_list {
    constraint cons;
    struct constraint_list* next;
    cJSON* json;
} constraint_list;

constraint_list* constraints_getFromFile(char*);
void constraints_prettyPrint(constraint_list*);
void constraints_freeAll(constraint_list*);

#endif //SOLVE_CONSTRAINTS_H

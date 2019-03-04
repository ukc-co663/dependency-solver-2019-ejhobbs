#ifndef SOLVE_CONSTRAINTS_H
#define SOLVE_CONSTRAINTS_H

#include "file.h"
#include "relation.h"

#define C_INSTALL '+'
#define C_REMOVE '-'
#define C_DEPEND '?'

#define N_INSTALL 1
#define N_REMOVE 2
#define N_DEPEND 4
typedef struct constraint {
  char op; /* install or remove */
  relation pkg;
  struct constraint *next;
} constraint;

typedef struct constraint_list {
  constraint *cons;
  cJSON *json;
} constraint_list;

constraint_list constraints_getFromFile(char *);
void constraints_printOp(char *c);
void constraints_prettyPrint(constraint *);
void constraints_freeAll(constraint_list *);

#endif // SOLVE_CONSTRAINTS_H

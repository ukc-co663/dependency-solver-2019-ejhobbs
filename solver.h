#include "repository.h"
#include "state.h"
#include "constraints.h"

#define KEEP    1
#define REMOVE  2

/**
 * bool_exp_list:
 * A
 * B -> !D -> F
 * !C
 */

typedef struct bool_exp {
  unsigned char option;
  package* pkg;
  struct bool_exp* next;
} bool_exp;

typedef struct bool_exp_list {
  bool_exp* exp;
  struct bool_exp_list* next;
} bool_exp_list;

bool_exp_list* solver_getRules(repository* repo, constraint_list* cs);
void solver_freeExpList(bool_exp_list*);

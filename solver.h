#include "repository.h"
#include "state.h"
#include "constraints.h"

/**
 * bool_conj:
 * A
 * B -> !D -> F
 * !C
 */

typedef struct bool_disj {
  char option;
  package* pkg;
  struct bool_disj* next;
} bool_disj;

typedef struct bool_conj {
  bool_disj* exp;
  struct bool_conj* next;
} bool_conj;

bool_conj* solver_getRules(repository* repo, constraint_list* cs);
constraint_list* solver_getConstraints(repository*, bool_conj*);
void solver_prettyPrint(bool_conj* exprs);
void solver_freeExpList(bool_conj*);

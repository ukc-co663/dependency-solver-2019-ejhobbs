#include "repository.h"
#include "state.h"
#include "constraints.h"
#include "relation.h"

#define PROC_UNIT 1
#define PROC_DUP  2
#define PROC_RED  4

/**
 * conj:
 * A
 * B -> !D -> F
 * !C
 */

typedef struct disj {
  char option;
  relation rel;
  package* pkg;
  struct disj* next;
} disj;

typedef struct conj {
  int processed;
  disj* exp;
  struct conj* next;
} conj;

conj* solver_getRules(repository* repo, constraint_list* cs);
constraint_list* solver_getConstraints(repository*, states*, conj*);
void solver_prettyPrint(conj* exprs);
void solver_freeExpList(conj*);

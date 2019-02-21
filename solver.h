#include "repository.h"
#include "state.h"
#include "constraints.h"

#define KEEP    1
#define REMOVE  2

typedef struct bool_exp {
  unsigned char option;
  package* pkg;
} bool_exp;

typedef struct bool_exp_list {
  bool_exp exp;
  bool_exp* next;
} bool_exp_list;

bool_exp_list solver_getRules(repository* repo, constraints* cs);

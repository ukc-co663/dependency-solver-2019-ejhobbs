#include "solver.h"

unsigned char opToChar(char* op) {
  switch (op) {
    case '+':
      return KEEP;
    case '-':
      return REMOVE;
  }
  return 0;
}

bool_exp_list solver_getRules(repository* repo, constraints* cs) {
  bool_exp_list rules = {{}};
  for (int i = 0; i < cs->size; i++) {
    bool_exp_list exprs = {{}};
    constraint c = cs->constraints[i];
    int idx = repo_getPackageIndex(repo, &c.pkg);
    package* pkg = repo->packages[idx];
    /* TODO add rule for this pkg */

    switch (c.op) {
      case '-':
        /* TODO don't need to add any of the dependencies, just create new rule for this */
        break;
      case '+':
        /* TODO add each dep as +constraint */
        break;
    }
  }
  return rules;
}

#include "solver.h"

unsigned char charToOp(char* op) {
  switch (*op) {
    case '+':
      return KEEP;
    case '-':
      return REMOVE;
  }
  return 0;
}

bool_exp_list* solver_getRules(repository* repo, constraints* cs) {
  bool_exp_list* rules = calloc(1, sizeof(&rules));
  for (int i = 0; i < cs->size; i++) {
    constraint c = cs->constraints[i];
    int idx = repo_getPackageIndex(repo, &c.pkg);
    package* cPkg = repo->packages[idx];

    /* Create rule for this package */
    bool_exp thisPkg = {charToOp(&c.op), cPkg};
    bool_exp_list* newRules = calloc(1, sizeof(&newRules));
    newRules->exp = thisPkg;
    newRules->next = rules;
    rules = newRules;

    switch (c.op) {
      case '-':
        /* TODO don't need to add any of the dependencies, just create new rule for this */
        break;
      case '+':
        /* TODO add each dep as +constraint and conflict as -constraint*/
        break;
    }
  }
  return rules;
}

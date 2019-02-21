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

bool_exp_list* solver_getRules(repository* repo, constraint_list* cs) {
  bool_exp_list* rules = NULL;
  while(cs != NULL) {
    constraint c = cs->cons;
    int idx = repo_getPackageIndex(repo, &c.pkg);
    package* cPkg = repo->packages[idx];

    /* Create rule for this package */
    bool_exp *thisPkg = calloc(1, sizeof(*thisPkg));
    thisPkg->option = charToOp(&c.op);
    thisPkg->pkg = cPkg;
    bool_exp_list* newRules = calloc(1, sizeof(*newRules));
    newRules->exp = thisPkg;
    newRules->next = rules;
    rules = newRules;

    switch (c.op) {
      case '-':
        /* don't need to add any of the dependencies, just create new rule for this */
        break;
      case '+':
        /* add each dep as +constraint */
        for (int i = 0; i < cPkg->cDepends; i++) {
          relation_group* thisRGrp = &cPkg->depends[i];
          bool_exp* thisGrp = NULL;
          /* create A v B v C v D statement */
          constraint_list* head = NULL;
          constraint_list* current = head;
          for (int j = 0; j < thisRGrp->size; j++) {
            int idx = repo_getPackageIndex(repo, &thisRGrp->relations[j]);
            package* thisPkg = repo->packages[idx];
            if (thisPkg->seen != 1) {
              thisPkg->seen = 1;

              bool_exp* newExp = calloc(1, sizeof(*newExp));
              newExp->option = KEEP;
              newExp->pkg = thisPkg;
              newExp->next = thisGrp;

              thisGrp = newExp;

              /* Create new constraint to deal with dependencies of this dependency */
              relation rel = {thisPkg->name, thisPkg->version, _eq};
              constraint newC = {C_INSTALL, rel};
              constraint_list* thisCons = calloc(1, sizeof(*thisCons));
              thisCons->cons = newC;
              if(head == NULL) {
                head = thisCons;
              }
              if (current != NULL) {
                current->next = thisCons;
              }
              current = thisCons;
            }
          }

          if (head != NULL) {
            /* we've got some dependencies to add to the queue */
            current->next = cs->next;
            cs->next = head;
          }

          bool_exp_list* newRules = calloc(1, sizeof(*newRules));
          newRules->exp = thisGrp;
          newRules->next = rules;
          rules = newRules;
        }
        /* add each conflict as -constraint*/
        break;
    }
    cs = cs->next;
  }
  return rules;
}

void solver_freeExpList(bool_exp_list* list) {
  while (list != NULL) {
    bool_exp_list* nxt = list->next;
    bool_exp* exp = list->exp;
    while(exp != NULL) {
      bool_exp* nxt = exp->next;
      free(exp);
      exp = nxt;
    }
    free(list);
    list = nxt;
  }
}

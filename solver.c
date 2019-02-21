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

bool_disj* newExpression(char inst, package* pkg) {
  bool_disj* exp = calloc(1, sizeof(*exp));
  exp->option = inst;
  exp->pkg = pkg;
  return exp;
}

constraint_list* constraintFromDependency(char op, relation* pkg) {
  constraint c = {op, *pkg};
  constraint_list* cl = calloc(1, sizeof(*cl));
  cl->cons = c;
  return cl;
}

bool_conj* solver_getRules(repository* repo, constraint_list* cs) {
  bool_conj* rules = NULL;
  while(cs != NULL) {
    constraint c = cs->cons;
    int idx = repo_getPackageIndex(repo, &c.pkg);
    package* cPkg = repo->packages[idx];

    /* Create rule for this package */
    bool_conj* newRules = calloc(1, sizeof(*newRules));
    newRules->exp = newExpression(charToOp(&c.op), cPkg);
    newRules->next = rules;
    rules = newRules;

    /* new constraints created by adding dependencies */
    constraint_list* head = NULL;
    constraint_list* current = head;
    switch (c.op) {
      case '-':
        /* nothing to do, we won't try uninstall things just because we don't need them */
        break;
      case '+':
        /* add each dep as +constraint */
        for (int i = 0; i < cPkg->cDepends; i++) {
          relation_group* deps = &cPkg->depends[i];
          bool_disj* thisGrp = NULL;
          /* create A v B v C v D statement */
          for (int j = 0; j < deps->size; j++) {
            /* repo, current, head -> return new current*/
            int idx = repo_getPackageIndex(repo, &deps->relations[j]);
            package* thisPkg = repo->packages[idx];
            if (idx >= 0 && !(thisPkg->seen & KEEP)) {
              thisPkg->seen &= KEEP;

              bool_disj* newExp = newExpression(KEEP, thisPkg);
              newExp->next = thisGrp;
              thisGrp = newExp;

              /* Create new constraint to deal with dependencies of this dependency */
              constraint_list* thisCons = constraintFromDependency(C_INSTALL, &deps->relations[j]);
              if(head == NULL) {
                head = thisCons;
              }
              if (current != NULL) {
                current->next = thisCons;
              }
              current = thisCons;
            }
          }

          bool_conj* newRules = calloc(1, sizeof(*newRules));
          newRules->exp = thisGrp;
          newRules->next = rules;
          rules = newRules;
        }
        /* add each conflict as -constraint*/
        for (int i = 0; i < cPkg->cConflicts; i++) {
          int idx = repo_getPackageIndex(repo, &cPkg->conflicts[i]);
          package* thisConflict = repo->packages[idx];
          if(idx >= 0 && !(thisConflict->seen & REMOVE)) {
            thisConflict->seen &= REMOVE;

            bool_conj* newRule = calloc(1, sizeof(*newRule));
            newRule->exp = newExpression(REMOVE, thisConflict);
            newRule->next = rules;
            rules = newRule;

            constraint_list* thisCons = constraintFromDependency(C_REMOVE, &cPkg->conflicts[i]);

            if (head == NULL) {
              head = thisCons;
            }
            if(current != NULL) {
              current->next = thisCons;
            }
            current = thisCons;
          }
        }
        break;
    }

    if (head != NULL) {
      /* we've got some dependencies to add to the queue */
      current->next = cs->next;
      cs->next = head;
    }
    cs = cs->next;
  }
  return rules;
}

void solver_prettyPrint(bool_conj* exprs) {
  while (exprs != NULL) {
    bool_disj* disj = exprs->exp;
    while (disj != NULL) {
      printf("%s", disj->pkg->name);
      printf("(");
      version_prettyPrint(&disj->pkg->version);
      printf(")");
      if(disj->next != NULL) { printf(" v "); }
      disj = disj->next;
    }
    printf("\n");
    exprs = exprs->next;
  }
}

void solver_freeExpList(bool_conj* list) {
  while (list != NULL) {
    bool_conj* nxt = list->next;
    bool_disj* exp = list->exp;
    while(exp != NULL) {
      bool_disj* nxt = exp->next;
      free(exp);
      exp = nxt;
    }
    free(list);
    list = nxt;
  }
}

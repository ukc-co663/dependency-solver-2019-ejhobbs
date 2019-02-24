#include "solver.h"

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

    if (!(c.op & N_DEPEND) || cPkg->cConflicts != 0 || cPkg->depends != 0) {
      /* Create rule for this package */
      bool_conj* newRules = calloc(1, sizeof(*newRules));
      newRules->exp = newExpression(c.op, cPkg);
      newRules->next = rules;
      rules = newRules;

      /* new constraints created by adding dependencies */
      constraint_list* head = NULL;
      constraint_list* current = head;
      if(c.op & N_INSTALL) {
          /* add each dep as +constraint */
          for (int i = 0; i < cPkg->cDepends; i++) {
            relation_group* deps = &cPkg->depends[i];
            bool_disj* thisGrp = NULL;
            /* create A v B v C v D statement */
            for (int j = 0; j < deps->size; j++) {
              /* repo, current, head -> return new current*/
              int idx = repo_getPackageIndex(repo, &deps->relations[j]);
              package* thisPkg = repo->packages[idx];
              if (idx >= 0) {
                if ((thisPkg->seen & N_INSTALL) == 0) {
                  thisPkg->seen |= N_INSTALL | N_DEPEND;

                  bool_disj* newExp = newExpression(N_INSTALL | N_DEPEND, thisPkg);
                  newExp->next = thisGrp;
                  thisGrp = newExp;
                }

                /* Create new constraint to deal with dependencies of this dependency */
                constraint_list* thisCons = constraintFromDependency(N_INSTALL | N_DEPEND, &deps->relations[j]);
                if(head == NULL) {
                  head = thisCons;
                }
                if (current != NULL) {
                  current->next = thisCons;
                }
                current = thisCons;
              }
            }

            if (thisGrp != NULL) {
              bool_disj* orNotThis = newExpression(N_REMOVE | N_DEPEND, cPkg);
              orNotThis->next = thisGrp;
              thisGrp = orNotThis;

              bool_conj* newRules = calloc(1, sizeof(*newRules));
              newRules->exp = thisGrp;
              newRules->next = rules;
              rules = newRules;
            }
          }
          /* add each conflict as -constraint*/
          for (int i = 0; i < cPkg->cConflicts; i++) {
            int idx = repo_getPackageIndex(repo, &cPkg->conflicts[i]);
            package* thisConflict = repo->packages[idx];
            if(idx >= 0) {
              thisConflict->seen |= N_REMOVE | N_DEPEND;

              bool_disj* notThis = newExpression(N_REMOVE | N_DEPEND, thisConflict);
              notThis->next = newExpression(N_REMOVE, cPkg);

              bool_conj* newRule = calloc(1, sizeof(*newRule));
              newRule->exp = notThis;
              newRule->next = rules;

              rules = newRule;

              constraint_list* thisCons = constraintFromDependency(N_REMOVE | N_DEPEND, &cPkg->conflicts[i]);

              if (head == NULL) {
                head = thisCons;
              }
              if(current != NULL) {
                current->next = thisCons;
              }
              current = thisCons;
            }
          }
      }

      if (head != NULL) {
        /* we've got some dependencies to add to the queue */
        current->next = cs->next;
        cs->next = head;
      }
    }
    cs = cs->next;
  }
  return rules;
}

constraint constraintFromRule(bool_disj* expr) {
  char cons_op = expr->option & ~(N_DEPEND);
  relation r = {expr->pkg->name, expr->pkg->version, _eq};
  return (constraint) {cons_op, r};
}

bool_disj* getMinimum(bool_disj* grp) {
  bool_disj* min = grp;
  while(grp != NULL) {
    if (grp->pkg->size < min->pkg->size) {
      min = grp;
    }
    grp = grp->next;
  }
  return min;
}

char reverseOperator(char op) {
  if (op & N_INSTALL) return N_REMOVE;
  if (op & N_REMOVE) return N_INSTALL;
  return 0;
}

bool_conj* simplifyRules(bool_conj* rules) {
  return rules;
}

bool_conj* removeContradictions(bool_conj* rules) {
  return rules;
}

constraint_list* solver_getConstraints(repository* repo, bool_conj* rules) {
  constraint_list* final = NULL;
  if (rules->next == NULL) {
    /* only one rule, can just install the smallest package and we're done */
    bool_disj* minExpr = getMinimum(rules->exp);
    constraint_list* thisRule = calloc(1, sizeof(*thisRule));
    thisRule->cons = constraintFromRule(minExpr);
    thisRule->next = final;
    final = thisRule;
  } else {
    bool_conj* simplified = simplifyRules(rules);
    bool_conj* noContradictions = removeContradictions(simplified);
    while (noContradictions != NULL) {
      bool_disj* minExpr = getMinimum(noContradictions->exp);
      constraint_list* thisRule = calloc(1, sizeof(*thisRule));
      thisRule->cons = constraintFromRule(minExpr);
      thisRule->next = final;
      final = thisRule;
      noContradictions = noContradictions->next;
    }
  }

  return final;
}

void solver_prettyPrint(bool_conj* exprs) {
  while (exprs != NULL) {
    bool_disj* disj = exprs->exp;
    while (disj != NULL) {
      constraints_printOp(&disj->option);
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

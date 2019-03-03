#include "solver.h"

disj* newExpression(char inst, package* pkg) {
  disj* exp = calloc(1, sizeof(*exp));
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

conj* solver_getRules(repository* repo, constraint_list* cs) {
  conj* rules = NULL;
  while(cs != NULL) {
    constraint c = cs->cons;
    int idx = repo_getPackageIndex(repo, &c.pkg);
    package* cPkg = repo->packages[idx];

    if (!(c.op & N_DEPEND) || cPkg->cConflicts != 0 || cPkg->depends != 0) {
      /* Create rule for this package */
      conj* newRules = calloc(1, sizeof(*newRules));
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
            disj* thisGrp = NULL;
            /* create A v B v C v D statement */
            for (int j = 0; j < deps->size; j++) {
              /* repo, current, head -> return new current*/
              int idx = repo_getPackageIndex(repo, &deps->relations[j]);
              package* thisPkg = repo->packages[idx];
              if (idx >= 0) {
                if ((thisPkg->seen & N_INSTALL) == 0) {
                  thisPkg->seen |= N_INSTALL | N_DEPEND;

                  disj* newExp = newExpression(N_INSTALL | N_DEPEND, thisPkg);
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
              disj* orNotThis = newExpression(N_REMOVE | N_DEPEND, cPkg);
              orNotThis->next = thisGrp;
              thisGrp = orNotThis;

              conj* newRules = calloc(1, sizeof(*newRules));
              newRules->processed = 0;
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

              disj* notThis = newExpression(N_REMOVE | N_DEPEND, thisConflict);
              notThis->next = newExpression(N_REMOVE, cPkg);
              notThis->rel = cPkg->conflicts[i];

              conj* newRule = calloc(1, sizeof(*newRule));
              newRule->processed = 0;
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

constraint constraintFromRule(disj* expr) {
  char cons_op = expr->option & ~(N_DEPEND); /* nand with N_DEPEND to remove markers that we use to see where packages are from */
  relation r = {expr->pkg->name, expr->pkg->version, _eq};
  return (constraint) {cons_op, r};
}

disj* getMinimum(disj* grp) {
  disj* min = grp;
  int minCost = 1000000000;
  while(grp != NULL) {
    int cost;
    if(grp->option & N_REMOVE) {
      cost = 1000000;
    } else {
      cost = grp->pkg->size;
    }
    if (cost < minCost) {
      minCost = cost;
      min = grp;
    }
    grp = grp->next;
  }
  return min;
}

disj reverseDisjunction(disj* d) {
  char newOp = 0;
  if (d->option & N_INSTALL) newOp = N_REMOVE;
  if (d->option & N_REMOVE) newOp = N_INSTALL;
  return (disj) {newOp, d->rel, d->pkg, d->next};
}

constraint_list* reverse(constraint_list* rules) {
  constraint_list* prev = NULL;
  constraint_list* next = NULL;
  constraint_list* cur = rules;
  while (cur != NULL) {
    next = cur->next;
    cur->next = prev;
    prev = cur;
    cur = next;
  }

  return prev;
}

void removeUnit(conj* rules, disj unit) {
  conj* thisRule = rules;
  while (thisRule != NULL) {
    disj* thisExpr = thisRule->exp;
    disj* prev = NULL;
    while (thisExpr != NULL) {
      if (thisExpr->pkg == unit.pkg && thisExpr->option & unit.option) {
        /* found a match, remove from list */
        if (prev == NULL) {
          /* in this case, we're at the top of the list. Thus, if next == NULL
           * then we've ended up passing in a unit clause which shouldn't
           * happen
           */
          if (thisExpr->next != NULL) {
            *thisExpr = *thisExpr->next;
          }
        } else {
          if (thisExpr->next != NULL) {
            *prev->next = *thisExpr->next;
            *thisExpr = *thisExpr->next;
          } else {
            prev->next = NULL;
            thisExpr = NULL;
          }
        }
      } else {
        prev = thisExpr;
        thisExpr = thisExpr->next;
      }
    }
    thisRule = thisRule->next;
  }
}

/**
 * Remove a single unit clause from subexpressions.
 * If there are no units to remove, return -1
 */
int findAndRemoveUnit(conj* rules) {
  int status = -1;
  conj* firstUnit = rules;
  while (status == -1 && firstUnit != NULL) {
    disj* thisExpr = firstUnit->exp;
    if (firstUnit->processed & PROC_UNIT && thisExpr->next == NULL) {
      /* found a unit */
      disj opposite = reverseDisjunction(thisExpr);
      removeUnit(rules, opposite);
      firstUnit->processed |= PROC_UNIT;
      status = 0;
    }
    firstUnit = firstUnit->next;
  }
  return status;
}

/**
 * Remove a single clause from subexpressions, and remove
 * toplevel duplicates (if they're units)
 */
int findAndRemoveDuplicate(conj* rules) {
  int status = -1;
  return status;
}

conj* removeContradictions(conj* rules) {
  return rules;
}

constraint_list* solver_getConstraints(repository* repo, states* currentState, conj* rules) {
  constraint_list* final = NULL;
  if (rules->next == NULL) {
    /* only one rule, can just install the smallest package and we're done */
    disj* minExpr = getMinimum(rules->exp);
    constraint_list* thisRule = calloc(1, sizeof(*thisRule));
    thisRule->cons = constraintFromRule(minExpr);
    thisRule->next = final;
    final = thisRule;
  } else {
    int hasUnits = 0;
    while (hasUnits != -1) {
      hasUnits = findAndRemoveUnit(rules);
    }
    int hasDups = 0;
    while(hasDups != -1) {
      hasDups = findAndRemoveDuplicate(rules);
    }
    while (rules != NULL) {
      disj* minExpr = getMinimum(rules->exp);
      constraint_list* thisRule = calloc(1, sizeof(*thisRule));
      thisRule->cons = constraintFromRule(minExpr);
      thisRule->next = final;
      final = thisRule;
      rules = rules->next;
    }
  }

  return reverse(final);
}

void solver_prettyPrint(conj* exprs) {
  while (exprs != NULL) {
    disj* disj = exprs->exp;
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

void solver_freeExpList(conj* list) {
  while (list != NULL) {
    conj* nxt = list->next;
    disj* exp = list->exp;
    while(exp != NULL) {
      disj* nxt = exp->next;
      free(exp);
      exp = nxt;
    }
    free(list);
    list = nxt;
  }
}

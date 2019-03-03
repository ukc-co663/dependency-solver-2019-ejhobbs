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
      disj* thisExpr = newExpression(c.op, cPkg);
      thisExpr->rel = c.pkg;
      newRules->exp = thisExpr;
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
                  newExp->rel = deps->relations[j];
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
              orNotThis->rel = c.pkg;
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
            if(idx >= 0) {
              disj* notThis = newExpression(N_REMOVE | N_DEPEND, NULL);
              notThis->rel = cPkg->conflicts[i];
              notThis->next = NULL;

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
  conj* thisRule = rules;
  while (thisRule != NULL) {
    thisRule = thisRule->next;
  }
  return status;
}

/**
 * When we have an uninstall command, there are two options:
 * a) the package is not installed, and so we don't care about it
 * b) the package is installed, and we must remove it.
 * In the second case, however, we don't know which package is actually installed,
 * since building the rules we don't store the package itself. Instead, we just
 * keep the relation, and work out if anything installed satisfies that constraint
 * and therefore needs removing.
 */
int tidyUninstalls(repository* repo, conj* r, states* s) {
  int status = -1;
  conj* thisRule = r;
  while (status == -1 && thisRule != NULL) {
    disj* thisExpr = thisRule->exp;
    if (thisExpr->option & N_REMOVE && thisExpr->option & N_DEPEND) {
      /* if something is installed already - create rule to remove it specifically */
      state_member* thisMember = s->members;
      int found = 0;
      while(thisMember != NULL) {
        if (strcmp(thisMember->rel.name, thisExpr->rel.name) == 0
            && relation_satisfiedByVersion(&thisMember->rel.version, &thisExpr->rel)){
          found = -1;
          disj* removeThis = calloc(1, sizeof(*removeThis));
          removeThis->next = thisExpr->next;
          removeThis->option = N_REMOVE;
          removeThis->pkg = repo->packages[repo_getPackageIndex(repo, &thisMember->rel)];
          *thisExpr = *removeThis;
        }
        thisMember = thisMember->next;
      }

      /* if not, discard this */
      if (found == 0) {
        *thisRule = *thisRule->next;
      } else {
        thisRule = thisRule->next;
      }
      status = 0;
    } else {
      thisRule = thisRule->next;
    }
  }
  return status;
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

    int fixUninstall = 0;
      //solver_prettyPrint(rules);
    while (fixUninstall != -1) {
      fixUninstall = tidyUninstalls(repo, rules, currentState);
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
    disj* thisDisj = exprs->exp;
    while (thisDisj != NULL) {
      constraints_printOp(&thisDisj->option);
      if (thisDisj->pkg == NULL) {
        printf("%s", thisDisj->rel.name);
        printf("(");
        relation_prettyPrint(&thisDisj->rel);
        printf(")");
      } else {
        printf("%s", thisDisj->pkg->name);
        printf("(");
        version_prettyPrint(&thisDisj->pkg->version);
        printf(")");
      }
      if(thisDisj->next != NULL) { printf(" v "); }
      thisDisj = thisDisj->next;
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

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

char opToChar(unsigned char* op) {
  switch (*op) {
    case KEEP:
      return '+';
    case REMOVE:
      return '-';
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
    if (!(cPkg->seen & KEEP) || !(cPkg->seen & REMOVE)) {
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
                thisPkg->seen |= KEEP;

                bool_disj* newExp = newExpression(KEEP | DEPEND, thisPkg);
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

            if (thisGrp != NULL) {
              bool_disj* orNotThis = newExpression(REMOVE, cPkg);
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
            if(idx >= 0 && !(thisConflict->seen & REMOVE)) {
              thisConflict->seen |= REMOVE;

              bool_disj* notThis = newExpression(REMOVE | DEPEND, thisConflict);
              notThis->next = newExpression(REMOVE, cPkg);

              bool_conj* newRule = calloc(1, sizeof(*newRule));
              newRule->exp = notThis;
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
    }
    cs = cs->next;
  }
  return rules;
}

void option_prettyPrint(unsigned char* c) {
  switch (*c) {
    case KEEP:
      printf("+");
      break;
    case REMOVE:
      printf("-");
      break;
  }
  if (*c & DEPEND) {
    printf("?");
  }
}

constraint_list* solver_getConstraints(repository* repo, bool_conj* rules) {
  constraint_list* final = NULL;
  if (rules->next == NULL) {
    bool_disj* expr = rules->exp;
    /* only one rule, can just create a constraint directly */
    constraint_list* thisRule = calloc(1, sizeof(*thisRule));
    char cons_op = opToChar(&expr->option);
    relation r = {expr->pkg->name, expr->pkg->version, _eq};
    constraint thisCons = {cons_op, r};
    thisRule->cons = thisCons;
    final = thisRule;
  }
  return final;
}

void solver_prettyPrint(bool_conj* exprs) {
  while (exprs != NULL) {
    bool_disj* disj = exprs->exp;
    while (disj != NULL) {
      option_prettyPrint(&disj->option);
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

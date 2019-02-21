#include "solver.h"

pkg_rule_list solver_getRules(repository* repo, constraints* cs) {
  pkg_rule_list rules = {{}};
  for (int i = 0; i < cs->size; i++) {
    pkg_rule rule = {0};
    bool_exp_list exprs = {{}};
    constraint c = cs->constraints[i];
    int idx = repo_getPackageIndex(repo, &c.pkg);
    package* pkg = repo->packages[idx];
    switch (c.op) {
      case '-':
        break;
      case '+':
        break;
    }
    rule.owner = &c;
    rule.expressions = exprs;
    pkg_rule_list newRule = {rule, &rules};
    rules = newRule;
  }
  return rules;
}

#include "repository.h"
#include "state.h"
#include "constraints.h"

typedef struct bool_exp {
  unsigned char option;
  package* pkg;
} bool_exp;

typedef struct bool_exp_list {
  bool_exp exp;
  bool_exp* next;
} bool_exp_list;

typedef struct pkg_rule {
  constraint* owner;
  bool_exp_list expressions;
} pkg_rule;

typedef struct pkg_rule_list {
  pkg_rule rule;
  pkg_rule* next;
} pkg_rule_list;

pkg_rule_list solver_getRules(repository* repo, constraints* cs);

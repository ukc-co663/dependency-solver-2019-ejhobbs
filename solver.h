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
}

typedef struct pkg_rule {
  package* owner;
  unsigned int expr_count;
  bool_exp_list expressions;
}

typedef struct pkg_rule_list {
  pkg_rule rule;
  pkg_rule* next;
}

pkg_rule_list get_final_state(repository* repo, constraints* cs);

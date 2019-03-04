#include "repository.h"
#include "state.h"
#include "constraints.h"
#include "relation.h"

#define _add_conflicts  1
#define _add_missing    2
#define _add_depends    4
/**
 * For storing dependencies, contains the options that we have
 * as well as the current 'best guess'.
 */
typedef struct d_path {
  int cur; /* which is our current guess */
  int size;
  relation* options; /* options we can choose */
} d_path;

typedef struct node {
  int src; /* -1 for root, idx for dependency */
  relation rel; /* what packages are we allowed */
  int pkg; /* what have we chosen currently (index into repo) */
  int numDepends;
  d_path* dependencies; /* Each dependency this package lists */
  struct node* next;
  int satisfied;
} node;

/**
 * We could have used relation_group here, but since this is going to
 * be changing a lot it's better to have it as a linked list instead
 */
typedef struct dep_list {
  int src; /* package where this came from */
  relation rel; /* thing we CAN'T have */
  struct dep_list* next;
} dep_list;

/**
 * Root node for our dependency graph. This will be passed around
 * and contains the route, and the things we have to avoid
 */
typedef struct option {
  node* route;
  dep_list* disallowed; /* things we're not allowed to install */
} option;

option solver_getRoute(const states*, const repository*, const constraint*);
constraint* solver_getConstraints(const repository*, const states*, const option*);

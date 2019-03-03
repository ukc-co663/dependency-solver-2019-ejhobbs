#include "solver.h"

constraint* list_append(constraint* l, constraint* r) {
  if (l == NULL) return r;
  r->next = l;
  return r;
}

disallowed* dis_append(disallowed* l, disallowed* r) {
  if (l == NULL) return r;
  r->next = l;
  return r;
}

constraint* maybeRemove(const states* s, relation* rel) {
  constraint* c = NULL;
  state_member* sm = s->members;
  while (sm != NULL) {
    if(strcmp(sm->rel.name, rel->name) == 0 && relation_satisfiedByVersion(&sm->rel.version, rel)) {
      constraint* nc = calloc(1, sizeof(*nc));
      nc->op = N_REMOVE;
      nc->pkg = *rel;
      c = list_append(c, nc);
    }
    sm = sm->next;
  }
  return c;
}


option solver_getRoute(const repository* repo, const constraint* cs) {
  node* startNode = NULL;
  disallowed* theseDisallowed = NULL;
  const constraint* thisCons = cs;
  while (thisCons != NULL) {
    if(thisCons->op & N_REMOVE) {
      disallowed* thisDisallowed = calloc(1, sizeof(*thisDisallowed));
      thisDisallowed->rel = thisCons->pkg;
      theseDisallowed = dis_append(theseDisallowed, thisDisallowed);
    }
    thisCons = thisCons->next;
  }
  return (option){startNode, theseDisallowed};
}

constraint* solver_getConstraints(const repository* repo, const states* state, const option* rules) {
  constraint* final = NULL;
  /* Make sure any disallowed are uninstalled FIRST. This ensures that
   * there can be no conflicts before we start installing
   */
  disallowed* thisConflict = rules->disallowed;
  while (thisConflict != NULL) {
    constraint* rm = maybeRemove(state, &thisConflict->rel);
    if (rm != NULL) {
      final = list_append(final, rm);
    }
    thisConflict = thisConflict->next;
  }
  /* Now we can traverse each of the nodes and install the packages */

  return final;
}

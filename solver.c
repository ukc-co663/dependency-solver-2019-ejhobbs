#include "solver.h"

constraint* list_append(constraint* l, constraint* r) {
  if (l == NULL) return r;
  r->next = l;
  return r;
}

dep_list* dis_append(dep_list* l, dep_list* r) {
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
  dep_list* theseDisallowed = NULL;
  const constraint* thisCons = cs;
  while (thisCons != NULL) {
    if(thisCons->op & N_REMOVE) {
      dep_list* thisDisallowed = calloc(1, sizeof(*thisDisallowed));
      thisDisallowed->rel = thisCons->pkg;
      theseDisallowed = dis_append(theseDisallowed, thisDisallowed);
    } else {
      // find first matching pkg
      // add its conflicts to disallowed (need to keep hold of them to remove later)
      // for each dependency group, pick the first. Recurse with each.
      // If conflict found, backtrack to choice that caused it.
      // error detection:
      // each pkg, check against disallowed. If any match, try the next best.
      // on backtrack, remove each set of conflicts from disallowed.
      // it's fine to have duplicates in conflicts. makes life easier.
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
  dep_list* thisConflict = rules->disallowed;
  while (thisConflict != NULL) {
    constraint* rm = maybeRemove(state, &thisConflict->rel);
    if (rm != NULL) {
      final = list_append(final, rm);
    }
    thisConflict = thisConflict->next;
  }
  /* Now we can traverse each of the nodes and install the packages */
  //TODO: traversal

  return final;
}

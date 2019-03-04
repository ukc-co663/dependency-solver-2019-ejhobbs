#include "solver.h"

/* these are all actually prepends */
node_list* nl_append(node_list* l, node_list* r) {
  if (l == NULL) return r;
  r->next = l;
  return r;
}

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

constraint* install(const repository* repo, int idx) {
  package* p = repo->packages[idx];
  constraint* c = calloc(1, sizeof(*c));
  c->op = N_INSTALL;
  c->pkg = (relation){p->name, p->version, _eq};
  return c;
}

int install_package(const states* s, const repository* repo, const constraint* c, node_list* start, node_list* this, dep_list* blocked) {
  int status = 0;
  // for each dependency group, pick the first. Recurse with each.
  // If conflict found, backtrack to choice that caused it.
  // error detection:
  // each pkg, check against disallowed. If any match, try the next best.
  // on backtrack, remove each set of conflicts from disallowed.
  // it's fine to have duplicates in conflicts. makes life easier.

  /* Check first to see if we can get away with not
   * installing a package if there are already
   * conflicts with a different version
   */
  int match = 0;
  int idx = repo_getPackageIndex(repo, &c->pkg);
  dep_list* cons = NULL;
  while (idx != 1 && match == 0) {
    constraint* found = NULL;
    cons = NULL;
    package* p = repo->packages[idx];
    for (int i = 0; i < p->cConflicts; i++) {
      relation rel = p->conflicts[i];
      constraint* installed = maybeRemove(s, &rel);
      dep_list* thisConflict = calloc(1, sizeof(*thisConflict));
      thisConflict->src = idx;
      thisConflict->rel = rel;
      found = list_append(found, installed);
      cons = dis_append(cons, thisConflict);
    }
    if (found != NULL) {
      idx = repo_getPackageFromIndex(repo, &c->pkg, idx+1);
      while (cons != NULL) {
        dep_list* nxt = cons->next;
        free(cons);
        cons = nxt;
      }
    } else {
      match = 1;
    }
  }
  /* If idx is -1 here, it means that we couldn't find a package with no
   * existing conflicts */
  if (idx == -1) {
    idx = repo_getPackageIndex(repo, &c->pkg);
    if (idx == -1) {
      status |= _add_missing;
    } else {
      package* p = repo->packages[idx];
      for (int i = 0; i < p->cConflicts; i++) {
        relation rel = p->conflicts[i];
        dep_list* thisConflict = calloc(1, sizeof(*thisConflict));
        thisConflict->src = idx;
        thisConflict->rel = rel;
        cons = dis_append(cons, thisConflict);
      }
    }
  }
  if (idx != -1) {
    /* add conflicts for the chosen version */
    if (cons != NULL) {
      *blocked = *cons;
      status |= _add_conflicts;
    }
    /* Deal with dependencies */
    package* chosen = repo->packages[idx];
    int dependencyCount = chosen->cDepends;
    d_path* depends = NULL;
    if (dependencyCount > 0) {
      depends = calloc(dependencyCount, sizeof(*depends));
      for (int i = 0; i < dependencyCount; i++) {
        relation_group thisPath = chosen->depends[i];
        depends[i].options = thisPath.relations;
        depends[i].cur = 0;
      }
    }
    this->pkg = (node){-1, c->pkg, idx, dependencyCount, depends};
  }
  return status;
}

option solver_getRoute(const states* s, const repository* repo, const constraint* cs) {
  node_list* startNode = NULL;
  dep_list* theseDisallowed = NULL;
  const constraint* thisCons = cs;
  while (thisCons != NULL) {
    if(thisCons->op & N_REMOVE) {
      dep_list* thisDisallowed = calloc(1, sizeof(*thisDisallowed));
      thisDisallowed->rel = thisCons->pkg;
      theseDisallowed = dis_append(theseDisallowed, thisDisallowed);
    } else {
      node_list* new = calloc(1, sizeof(*new));
      dep_list* block = calloc(1, sizeof(*block));
      int status = install_package(s, repo, thisCons, startNode, new, block);
      if (status & _add_missing) {
        free(new);
        free(block);
      } else {
        if (status & _add_conflicts) {
          theseDisallowed = dis_append(theseDisallowed, block);
        } else {
          free(block);
        }
        startNode = nl_append(startNode, new);
      }
    }
    thisCons = thisCons->next;
  }
  return (option){startNode, theseDisallowed};
}


constraint* solver_getConstraints(const repository* repo, const states* state, const option* rules) {
  constraint* final = NULL;
  //TODO: traversal
  node_list* toInstall = rules->route;
  while(toInstall != NULL) {
    constraint* ins = install(repo, toInstall->pkg.pkg);
    final = list_append(final, ins);
    toInstall = toInstall->next;
  }

  /* Make sure any disallowed are uninstalled first. This ensures that
   * there can be no conflicts before we start installing. Since this
   * is a LL, we do this last as we push to the front.
   */
  dep_list* thisConflict = rules->disallowed;
  while (thisConflict != NULL) {
    constraint* rm = maybeRemove(state, &thisConflict->rel);
    if (rm != NULL) {
      final = list_append(final, rm);
    }
    thisConflict = thisConflict->next;
  }
  return final;
}

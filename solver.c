#include "solver.h"

/* these are all actually prepends */
node *nl_append(node *l, node *r) {
  if (l == NULL)
    return r;
  node *tmp = r;
  while (tmp->next != NULL)
    tmp = tmp->next;
  tmp->next = l;
  return r;
}

constraint *list_append(constraint *l, constraint *r) {
  if (l == NULL)
    return r;
  constraint *tmp = r;
  while (tmp->next != NULL)
    tmp = tmp->next;
  tmp->next = l;
  return r;
}

dep_list *dis_append(dep_list *l, dep_list *r) {
  if (l == NULL)
    return r;
  dep_list *tmp = r;
  while (tmp->next != NULL)
    tmp = tmp->next;
  tmp->next = l;
  return r;
}

constraint *maybeRemove(const states *s, relation *rel) {
  constraint *c = NULL;
  state_member *sm = s->members;
  while (sm != NULL) {
    if (strcmp(sm->rel.name, rel->name) == 0 &&
        relation_satisfiedByVersion(&rel->version, &sm->rel)) {
      constraint *nc = calloc(1, sizeof(*nc));
      nc->op = N_REMOVE;
      nc->pkg = sm->rel;
      c = list_append(c, nc);
    }
    sm = sm->next;
  }
  return c;
}

constraint *install(const repository *repo, int idx) {
  package *p = repo->packages[idx];
  constraint *c = calloc(1, sizeof(*c));
  c->op = N_INSTALL;
  c->pkg = (relation){p->name, p->version, _eq};
  return c;
}

dep_list *formatConflicts(const repository *r, int idx) {
  package *chosen = r->packages[idx];
  int numConflicts = chosen->cConflicts;
  dep_list *conflicts = NULL;
  if (numConflicts > 0) {
    for (int i = 0; i < numConflicts; i++) {
      dep_list *thisConflict = calloc(1, sizeof(*thisConflict));
      thisConflict->src = idx;
      thisConflict->rel = chosen->conflicts[i];
      thisConflict->next = conflicts;
      conflicts = thisConflict;
    }
  }
  return conflicts;
}

d_path *formatDependencies(const repository *r, int idx) {
  package *chosen = r->packages[idx];
  int dependencyCount = chosen->cDepends;
  d_path *depends = NULL;
  if (dependencyCount > 0) {
    depends = calloc(dependencyCount, sizeof(*depends));
    for (int i = 0; i < dependencyCount; i++) {
      relation_group thisPath = chosen->depends[i];
      depends[i].options = thisPath.relations;
      depends[i].size = thisPath.size;
      depends[i].cur = 0;
    }
  }
  return depends;
}

int install_package(const states *s, const repository *repo,
                    const constraint *c, node *start, node *this,
                    dep_list *blocked) {
  int status = 0;
  /* Try to find a copy of the constraint that doesn't require
   * us to uninstall something right off the bat. This
   * should reduce the cost a bit in the long run.
   */
  int match = 0;
  int idx = repo_getPackageIndex(repo, &c->pkg);
  dep_list *cons = NULL;
  while (idx != -1 && match == 0) {
    int found = 0;
    package *p = repo->packages[idx];
    for (int i = 0; i < p->cConflicts; i++) {
      relation rel = p->conflicts[i];
      constraint *installed = maybeRemove(s, &rel);
      if (installed != NULL) {
        free(installed);
        found = 1;
        break;
      }
    }
    if (found != 0) {
      idx = repo_getPackageFromIndex(repo, &c->pkg, idx + 1);
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
      package *p = repo->packages[idx];
      for (int i = 0; i < p->cConflicts; i++) {
        relation rel = p->conflicts[i];
        dep_list *thisConflict = calloc(1, sizeof(*thisConflict));
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
    d_path *depends = formatDependencies(repo, idx);
    this->src = -1;
    this->rel = c->pkg;
    this->pkg = idx;
    this->numDepends = repo->packages[idx]->cDepends;
    this->dependencies = depends;
  }
  return status;
}

int isBlocked(dep_list *bs, relation *rel) {
  dep_list *tb = bs;
  while (tb != NULL) {
    if (strcmp(tb->rel.name, rel->name) == 0 &&
        relation_satisfiedByVersion(&tb->rel.version, rel)) {
      return 1;
    }
    tb = tb->next;
  }
  return 0;
}

option resolveDepends(const states *s, const repository *r, node *start,
                      dep_list *block) {
  if (start != NULL) {
    if (start->numDepends != 0) {
      int set = 0;
      while (set < start->numDepends) {
        d_path thisPath = start->dependencies[set];
        relation tryRel = thisPath.options[thisPath.cur];
        if (isBlocked(block, &tryRel)) {
          if (thisPath.cur++ >= thisPath.size) {
            return (option){
                NULL,
                NULL}; // tell previous call we cannot satisfy some dependency
          }
        } else {
          int idx = repo_getPackageIndex(r, &tryRel);
          node *dep = calloc(1, sizeof(*dep));
          d_path *subdeps = formatDependencies(r, idx);
          dep->src = start->pkg;
          dep->rel = tryRel;
          dep->pkg = idx;
          dep->numDepends = r->packages[idx]->cDepends;
          dep->dependencies = subdeps;
          dep->next = start->next; // insert out package afterward
          start->next = dep;
          dep_list *subconflicts = formatConflicts(r, idx);
          if (subconflicts != NULL)
            block = dis_append(block, subconflicts);
          set++; // try next set
        }
      }
    }
    if (start->next != NULL) {
      option nxt = resolveDepends(s, r, start->next, block);
      if (nxt.route == NULL) {
        // something went wrong, try again
        // also need to tidy up here first.
        if (block != NULL)
          block->next = NULL; /* unblock things we chose */
        /* also need to remove the dependencies we added */
        return resolveDepends(s, r, start, block);
      }
    }
  }
  return (option){start, block};
}

option solver_getRoute(const states *s, const repository *repo,
                       const constraint *cs) {
  node *startNode = NULL;
  dep_list *blocked = NULL;
  const constraint *thisCons = cs;
  while (thisCons != NULL) {
    if (thisCons->op & N_REMOVE) {
      dep_list *thisDisallowed = calloc(1, sizeof(*thisDisallowed));
      if (thisDisallowed != NULL) {
        thisDisallowed->rel = thisCons->pkg;
        blocked = dis_append(blocked, thisDisallowed);
      }
    } else {
      node *new = calloc(1, sizeof(*new));
      dep_list *block = calloc(1, sizeof(*block));
      int status = install_package(s, repo, thisCons, startNode, new, block);
      startNode = nl_append(startNode, new);
      if (status & _add_conflicts) {
        blocked = dis_append(blocked, block);
      } else {
        free(block);
      }
      // for each dependency group, pick the first. Recurse with each.
      // If conflict found, backtrack to choice that caused it.
      // error detection:
      // each pkg, check against disallowed. If any match, try the next best.
      // on backtrack, remove each set of conflicts from disallowed.
      // it's fine to have duplicates in conflicts. makes life easier.
    }
    thisCons = thisCons->next;
  }

  return resolveDepends(s, repo, startNode, blocked);
}

node* remove_duplicates(node* n, int pkg) {
  //Wrong, have to go the other way around :s
  node* nc = n;
  node* prev = NULL;
  while( nc != NULL) {
    if (nc->pkg == pkg) {
      if (prev != NULL) {
        prev->next = nc->next;
        nc = nc->next;
      } else {
        nc = nc->next;
      }
    } else {
      prev = nc;
      nc = nc->next;
    }
  }
  return n;
}

constraint *solver_getConstraints(const repository *repo, const states *state,
                                  const option *rules) {
  constraint *final = NULL;
  node *toInstall = rules->route;
  while (toInstall != NULL) {
    constraint *ins = install(repo, toInstall->pkg);
    final = list_append(final, ins);
    // TODO ignore duplicates
    toInstall = toInstall->next; //remove_duplicates(toInstall->next, toInstall->pkg);
  }

  /* Make sure any disallowed are uninstalled first. This ensures that
   * there can be no conflicts before we start installing. Since this
   * is a LL, we do this last as we push to the front.
   */
  dep_list *thisConflict = rules->disallowed;
  while (thisConflict != NULL) {
    constraint *rm = maybeRemove(state, &thisConflict->rel);
    if (rm != NULL) {
      final = list_append(final, rm);
    }
    thisConflict = thisConflict->next;
  }
  return final;
}

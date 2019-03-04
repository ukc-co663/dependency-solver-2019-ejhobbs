#include "relation.h"

relation *relation_getAll(const cJSON *relationList, int count) {
  relation *relations = NULL;
  if (count > 0) {
    relations = calloc((size_t)count, sizeof(relation));
    cJSON *rel = NULL;
    int i = 0;
    cJSON_ArrayForEach(rel, relationList) {
      relations[i] = parseRelation(rel->valuestring);
      i++;
    }
  }
  return relations;
}

relation parseRelation(char *rel) {
  int sName = 0; /* Size of pkg name */
  int comp = 0;  /* Start with any */
  int sym = 0;
  char *curChar = rel;
  /* Loop over the string until we get to the first symbol, ie end of the name*/
  while (sName < strlen(rel) && !sym) {
    switch (*curChar) {
    case '<':
    case '>':
    case '=':
      sym = 1;
      break;
    default:
      sName++;
      break;
    }
    if (!sym) {
      curChar++;
    }
  }
  char *name = calloc((size_t)sName + 1, sizeof(char));
  strncpy(name, rel, sName);
  name[sName] = 0;
  int num = 0;
  while (curChar < rel + strlen(rel) && !num) {
    switch (*curChar) {
    case '<':
      comp |= _lt;
      break;
    case '>':
      comp |= _gt;
      break;
    case '=':
      comp |= _eq;
      break;
    default:
      num = 1;
      break;
    }
    if (!num) {
      curChar++;
    }
  }
  version ver = {0, NULL};
  if (num) {
    ver = parseVersion(curChar);
  }

  relation newRel = {name, ver, comp};
  return newRel;
}

version parseVersion(char *versionString) {
  int *parts = NULL;
  int vSize = 0;
  char delim[] = ".";
  char *part = strtok(versionString, delim);
  if (part == NULL) {
    parts = malloc(sizeof(int));
    vSize = 1;
    *parts = atoi(versionString);
  } else {
    while (part != NULL) {
      int *tmp = realloc(parts, (vSize + 1) * sizeof(int));
      if (tmp != NULL) {
        parts = tmp;
        parts[vSize] = atoi(part);
        part = strtok(NULL, delim);
        vSize++;
      } else {
        perror("Something went wrong");
      }
    }
  }
  version vers = {vSize, parts};
  return vers;
}

void version_prettyPrint(version *v) {
  for (int i = 0; i < v->size; i++) {
    printf("%d", v->val[i]);
    if (i < v->size - 1) {
      printf(".");
    }
  }
}

void comp_prettyPrint(char *c) {
  if (*c & _gt)
    printf(">");
  if (*c & _lt)
    printf("<");
  if (*c & _eq)
    printf("=");
}

void relation_prettyPrint(relation *r) {
  printf("%s", r->name);
  comp_prettyPrint(&r->comp);
  version_prettyPrint(&r->version);
}

void relation_freeAll(int s, relation *r) {
  for (int i = 0; i < s; i++) {
    free(r[i].name);
    version_free(&(r[i].version));
  }
  free(r);
}

void relation_free(relation *r) {
  free(r->name);
  version_free(&(r->version));
}

void version_free(version *v) {
  if (v->size > 0) {
    free(v->val);
  }
}

int compareInner(version *, version *, int, int);
int relation_compareVersion(version *v1, version *v2) {
  if (v1->size == 0 || v2->size == 0) {
    return 0;
  }
  if (v1->size > v2->size) {
    return compareInner(v1, v2, 0, v2->size);
  }
  return compareInner(v1, v2, 0, v1->size);
}

int allZero(version *, int);
/**
 * Compares two versions recursively, returning when one is clearly
 * larger/smaller than the other
 * @param v1
 * @param v2
 * @param idx current index
 * @return if equal 0, else positive for v1 larger and negative for v1 smaller
 */
int compareInner(version *v1, version *v2, int idx, int max) {
  if (idx >= max) {
    // when we get to the end of the smaller version, there may still be more
    // yet in the larger, and we need to check that final one
    version *toCheck;
    if (v1->size <= max) {
      toCheck = v2;
    } else {
      toCheck = v1;
    }
    if (allZero(toCheck, idx)) {
      return 0; // same
    }
    if (v1->size > v2->size) {
      return 1;
    } else {
      return -1;
    }

  } else {
    int diff = v1->val[idx] - v2->val[idx];
    if (diff > 0 || diff < 0) {
      return diff;
    } else {
      return compareInner(v1, v2, idx + 1, max);
    }
  }
}

int relation_satisfiedByVersion(version *v, relation *r) {
  if (r->version.size > 0) {
    int relationship = relation_compareVersion(v, &r->version);
    return relation_satisfiesConstraint(relationship, r->comp);
  }
  return 1;
}

int relation_satisfiesConstraint(int c, char comp) {
  if (comp == 0) {
    return 1;
  }
  if (comp & _eq && c == 0) {
    return 1;
  }
  if (comp & _lt && c < 0) {
    return 1;
  }
  if (comp & _gt && c > 0) {
    return 1;
  }
  return 0;
}

int allZero(version *v, int start) {
  int highest = 0;
  for (int i = start; i < v->size; i++) {
    if (v->val[i] > highest) {
      highest = v->val[i];
    };
  }
  return highest == 0;
}

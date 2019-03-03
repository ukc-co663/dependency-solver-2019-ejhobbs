#include "repository.h"

package* package_fromJson(const cJSON*);

void getAllDependencies(package*, const cJSON*);
relation* relation_getAll(const cJSON *relationList, int count);
int comparePkg(const void* p, const void* q) {
    const package* l = *(const package**)p;
    const package* r = *(const package**)q;

    int strRes = strcmp(l->name, r->name);
    if(strRes != 0) {
      return strRes;
    }
    int sizeDiff = l->size - r->size;
    if(sizeDiff != 0) {
      return sizeDiff;
    }
    return relation_compareVersion(&l->version, &r->version);
}

repository repo_getFromFile(const char* f) {
    repository repo = {0, NULL};
    char* fContents = getFullContents(f);

    cJSON *parsedJson = cJSON_Parse(fContents);
    if (parsedJson == NULL) {
        const char *e_ptr = cJSON_GetErrorPtr();
        if(e_ptr != NULL) {
            fprintf(stderr, "Repository: JSON parse error: %s\n", e_ptr);
        }
    } else {
        /* Do some stuff */
        if (!cJSON_IsArray(parsedJson)) {
            fprintf(stderr, "Repository expected array, got %#x!\n", parsedJson->type);
            cJSON_Delete(parsedJson);
        } else {
            repo = repo_getAll(parsedJson);
        }
    }
    free(fContents);
    return repo;
}

repository repo_getAll(const cJSON *repo) {
    int numPkgs = cJSON_GetArraySize(repo);
    package** availablePkgs = calloc(numPkgs, sizeof(*availablePkgs));

    /* Go from Json layout to array of struct*s */
    package** curPackage = availablePkgs;
    const cJSON *pkg = NULL;
    cJSON_ArrayForEach(pkg, repo) {
        *curPackage = package_fromJson(pkg);
        curPackage++;
    }
    /* Sort result for faster finds */
    qsort((void*)availablePkgs, numPkgs, sizeof(package*), comparePkg);
    repository group = {numPkgs, availablePkgs, repo};
    return group;
}

package* package_fromJson(const cJSON* jsonPkg){
  cJSON* name = cJSON_GetObjectItemCaseSensitive(jsonPkg, "name");
  cJSON* size = cJSON_GetObjectItemCaseSensitive(jsonPkg, "size");
  cJSON* version = cJSON_GetObjectItemCaseSensitive(jsonPkg, "version");
  cJSON* depends = cJSON_GetObjectItemCaseSensitive(jsonPkg, "depends");
  cJSON* conflicts = cJSON_GetObjectItemCaseSensitive(jsonPkg, "conflicts");

  /* Check everything's in order */
  if (!cJSON_IsNumber(size) || !cJSON_IsString(name) ||
      !cJSON_IsString(version)) {
    fprintf(stderr, "Malformed package information\n");
    return NULL;
  }
  package* parsed = calloc(1, sizeof(package));
  parsed->name = name->valuestring;
  parsed->size = size->valueint;
  parsed->version = parseVersion(version->valuestring);

  int numConflicts = 0;

  if (depends != NULL && cJSON_IsArray(depends)) {
    getAllDependencies(parsed, depends);
  }
  if (conflicts != NULL && cJSON_IsArray(conflicts)) {
    numConflicts = cJSON_GetArraySize(conflicts);
    parsed->conflicts = relation_getAll(conflicts, numConflicts);
    parsed->cConflicts = numConflicts;
  }

  return parsed;
}

void getAllDependencies(package* pkg, const cJSON* deps) {
    int numGroups = cJSON_GetArraySize(deps);
    relation_group* groups = NULL;
    if (numGroups > 0) {
        groups = calloc((size_t) numGroups, sizeof(relation_group));
        cJSON* thisItem = NULL;
        int curGroup = 0;
        cJSON_ArrayForEach(thisItem, deps) {
            int thisSize = cJSON_GetArraySize(thisItem);
            relation* theseRelations = relation_getAll(thisItem, thisSize);

            relation_group grp = {thisSize, theseRelations};
            groups[curGroup] = grp;
            curGroup++;
        }
    }
  pkg->cDepends = numGroups;
  pkg->depends = groups;
}

void package_free(package*);

void repo_freeAll(repository* pg) {
    for (int i=0; i < pg->size; i++) {
        if(pg->packages[i] != NULL) {
            package_free(pg->packages[i]);
        }
    }
    free(pg->packages);
    cJSON_Delete(pg->json);
}

int compareNames(const repository* r, char* n, int i) {
  return strcmp(n, r->packages[i]->name);
}

int satisfiesConstraint(const repository* r, relation* rl, int i) {
  return relation_satisfiedByVersion(&r->packages[i]->version, rl);
}
/* Linear search through the whole repository. NOT the most efficient by any
 * stretch, but it does guarantee that we will always get the minimal cost
 * package for a given constraint
 */
int repo_getPackageIndex(const repository* r, relation* rl) {
  return repo_getPackageFromIndex(r, rl, 0);
}
int repo_getPackageFromIndex(const repository *repo, relation* r, int idx) {
  int max = repo->size;

  /* Increase until we find a matching name */
  while(idx < max && compareNames(repo, r->name, idx) > 0) {
    idx++;
  };

  if(compareNames(repo, r->name, idx) != 0) return -1; /* went past where it should have been */

  /* Increase until we find the first pkg to satisfy the constraint */
  while(idx < max && compareNames(repo, r->name, idx) == 0 && satisfiesConstraint(repo, r, idx) != 1) {
    idx++;
  }

  if(compareNames(repo, r->name, idx) != 0) return -1; /* went past where it should have been */

  return idx;
}

void package_free(package* p) {
    if (p->cDepends > 0 && p->depends != NULL) {
        for(int i = 0; i < p->cDepends; i++) {
            relation_freeAll(p->depends[i].size, p->depends[i].relations);
        }
        free(p->depends);
    }
    if (p->cConflicts > 0 && p->conflicts != NULL) relation_freeAll(p->cConflicts, p->conflicts);
    version_free(&(p->version));
    free(p);
}

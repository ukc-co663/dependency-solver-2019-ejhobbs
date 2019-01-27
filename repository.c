#include "repository.h"

int repo_getPackageIndex(const repo_repository * grp, const char *name) {
    int idx = -1;
    int l = 0;
    int r = grp->size-1;
    while (idx < 0 && l <= r) {
        int mid = l + (r-l)/2;
        int cmp = strcmp(name, grp->packages[mid]->name);
        if (cmp == 0) {
            idx = mid;
        } else if (cmp > 0) {
            l = mid+1;
        } else {
            r = mid-1;
        }
    }
    return idx;
}

package* package_fromJson(const cJSON*);
void getAllDependencies(package*, const cJSON*);
relation* getAllRelations(const cJSON*, int);

int comparePkg(const void* p, const void* q) {
    const package* l = *(const package**)p;
    const package* r = *(const package**)q;

    return strcmp(l->name, r->name);
}

repo_repository repo_getFromFile(const char* f) {
    repo_repository repo = {0, NULL};
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
        } else {
            repo = repo_getAll(parsedJson);
        }
    }
    free(fContents);
    return repo;
}

repo_repository repo_getAll(const cJSON *repo) {
    int numPkgs = cJSON_GetArraySize(repo);
    package** availablePkgs = malloc(numPkgs * sizeof(package*));

    /* Go from Json layout to array of struct*s */
    package** curPackage = availablePkgs;
    const cJSON *pkg = NULL;
    cJSON_ArrayForEach(pkg, repo) {
        *curPackage = package_fromJson(pkg);
        curPackage++;
    }
    /* Sort result for faster finds */
    qsort((void*)availablePkgs, numPkgs, sizeof(package*), comparePkg);
    repo_repository group = {numPkgs, availablePkgs, repo};
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
    parsed->conflicts = getAllRelations(conflicts, numConflicts);
    parsed->cConflicts = numConflicts;
  }

  return parsed;
}

void getAllDependencies(package* pkg, const cJSON* deps) {
  int numGroups = cJSON_GetArraySize(deps);
  relation_group* groups = calloc((size_t) numGroups, sizeof(relation_group));
  cJSON* thisItem = NULL;
  int curGroup = 0;
  cJSON_ArrayForEach(thisItem, deps) {
    int thisSize = cJSON_GetArraySize(thisItem);
    relation* theseRelations = getAllRelations(thisItem, thisSize);

    relation_group grp = {thisSize, theseRelations};
    groups[curGroup] = grp;
    curGroup++;
  }
  pkg->cDepends = numGroups;
  pkg->depends = groups;
}

void package_free(package*);
void relation_free(int, relation*);
void version_free(version*);

void repo_freeAll(repo_repository pg) {
    for (int i=0; i < pg.size; i++) {
        if(pg.packages[i] != NULL) {
            package_free(pg.packages[i]);
        }
    }
    free(pg.packages);
    cJSON_Delete(pg.json);
}

void package_free(package* p) {
    if (p->cDepends > 0 && p->depends != NULL) {
        for(int i = 0; i < p->cDepends; i++) {
            relation_free(p->depends[i].size, p->depends[i].relations);
        }
        free(p->depends);
    }
    if (p->cConflicts > 0 && p->conflicts != NULL) relation_free(p->cConflicts, p->conflicts);
    version_free(&(p->version));
    free(p);
}

void relation_free(int s, relation* r) {
  for (int i = 0; i < s; i++) {
    free(r[i].name);
    version_free(&(r[i].version));
  }
  free(r);
}

void version_free(version* v) {
  if (v->size > 0) {
    free(v->val);
  }
}
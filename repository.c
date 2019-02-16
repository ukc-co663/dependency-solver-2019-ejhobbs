#include "repository.h"

package* package_fromJson(const cJSON*);

void getAllDependencies(package*, const cJSON*);
relation* relation_getAll(const cJSON *relationList, int count);
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
            cJSON_Delete(parsedJson);
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

void repo_freeAll(repo_repository* pg) {
    for (int i=0; i < pg->size; i++) {
        if(pg->packages[i] != NULL) {
            package_free(pg->packages[i]);
        }
    }
    free(pg->packages);
    cJSON_Delete(pg->json);
}

int repo_getPackageIndex(const repo_repository *repo, const char *name, version *v) {
    int idx = 0;
    // while (idx in range AND (name does not match AND first char of repo pkg is < first char of name))
    while(idx < repo->size-1 && (strcmp(repo->packages[idx]->name, name) != 0 && *(repo->packages[idx]->name) < *name)){
        /* Increase position in array until either we find a name match, or we go above where it should be */
        idx += 1;
    }
    if (strcmp(repo->packages[idx]->name, name) != 0){
        /* Went past where the package should have been */
        return -1;
    }
    if (v->size > 0) {
      while(idx < repo->size-1 && (strcmp(repo->packages[idx]->name, name) == 0)) {
          /* Increase position until we find a version match, or another package */
        if (relation_compareVersion(&(repo->packages[idx]->version), v) == 0) {
          return idx;
        } else {
          idx++;
        }
      }
      return -1;
    } else {
      return idx;
    }
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

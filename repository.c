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

int repo_getPackageIndex(const repository *repo, relation* r) {
    int idx = -1;
    int left = 0;
    int right = repo->size-1;
    while (idx < 0 && left <= right) {
        int mid = left + (right-left)/2;
        int cmp = strcmp(r->name, repo->packages[mid]->name);
        if (cmp == 0) {
            int vcmp = relation_compareVersion(&repo->packages[mid]->version, &r->version);
            if (relation_satisfiesConstraint(vcmp, r->comp) == 1) {
              idx = mid;
            } else if (vcmp > 0) {
              left = mid+1;
            } else {
              right = mid-1;
            }
        } else if (cmp > 0) {
            left = mid+1;
        } else {
            right = mid-1;
        }
    }
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

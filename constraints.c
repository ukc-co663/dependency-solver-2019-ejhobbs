#include "constraints.h"

constraint parseConstraint(char *c);

constraints constraints_getFromFile(char* f) {

    constraints cs = {0, NULL, NULL};

    char* fContents = getFullContents(f);
    cJSON* json = cJSON_Parse(fContents);

    if(json == NULL) {
        const char *e_ptr = cJSON_GetErrorPtr();
        if (e_ptr != NULL) {
            fprintf(stderr, "Constraints: JSON parse error: %s\n", e_ptr);
        }
    } else {
        if(!cJSON_IsArray(json)) {
            fprintf(stderr, "Constraints expected array, got %#x!\n",json->type);
            cJSON_Delete(json);
        } else {
            int numItems = cJSON_GetArraySize(json);
            constraint* items = calloc(numItems, sizeof(constraint));
            constraint* thisItem = items;
            cJSON* thisConstraint = NULL;
            cJSON_ArrayForEach(thisConstraint, json) {
                *thisItem = parseConstraint(thisConstraint->valuestring);
                thisItem++;
            }
            cs = (constraints) {numItems, items, json};
        }
    }
    free(fContents);
    return cs;
}

constraint parseConstraint(char *c) {
    constraint result;
    result.op = *c;
    result.pkg = parseRelation(c+1); /* Constraints consist of +/- then a package then eq then version */
    return result;
}

void constraints_freeAll(constraints* cs) {
    for (int i=0; i < cs->size; i++) {
        constraint c = cs->constraints[i];
        free(c.pkg.name);
        version_free(&c.pkg.version);
    }
    free(cs->constraints);
    cJSON_Delete(cs->json);
}
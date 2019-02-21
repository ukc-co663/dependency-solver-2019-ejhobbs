#include "constraints.h"

constraint parseConstraint(char *c);

constraint_list* constraints_getFromFile(char* f) {

    constraint_list* cs = NULL;

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
            cJSON* thisConstraint = NULL;
            cJSON_ArrayForEach(thisConstraint, json) {
              constraint_list* newCs = calloc(1, sizeof(*newCs));
              newCs->cons = parseConstraint(thisConstraint->valuestring);
              newCs->next = cs;
              cs = newCs;
            }
        }
    }
    if (cs != NULL) {
      cs->json = json;
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

void csonstraints_prettyPrint(constraint_list* cs) {
  printf("[");
  while (cs != NULL) {
    printf("\"");
    printf("%c", cs->cons.op);
    relation_prettyPrint(&cs->cons.pkg);
    printf("\"");
    if(cs->next != NULL) {
      printf(",");
    }
    cs = cs->next;
  }
  printf("]\n");
}

void constraints_freeAll(constraint_list* cs) {
  while (cs != NULL) {
    constraint_list* next = cs->next;
    free(cs->cons.pkg.name);
    version_free(&cs->cons.pkg.version);
    if (cs->json != NULL) {
      cJSON_Delete(cs->json);
    }
    free(cs);
    cs = next;
  }
}

#include "constraints.h"

constraint* parseConstraint(char *c);
constraint_list constraints_getFromFile(char* f) {
    constraint* c = NULL;

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
              constraint* newc = parseConstraint(thisConstraint->valuestring);
              newc->next = c;
              c = newc;
            }
        }
    }
    free(fContents);
    return (constraint_list){c, json};
}

char charToOp(char* c) {
  switch (*c) {
    case C_INSTALL:
      return N_INSTALL;
    case C_REMOVE:
      return N_REMOVE;
  }
  return 0;
}

constraint* parseConstraint(char *c) {
  constraint *cons = calloc(1, sizeof(*cons));
  cons->op = charToOp(c);
  cons->pkg = parseRelation(c+1); /* Constraints consist of +/- then a package then eq then version */
  return cons;
}

void constraints_printOp(char* op) {
  if (*op & N_INSTALL) printf("%c", C_INSTALL);
  if (*op & N_REMOVE) printf("%c", C_REMOVE);
  if (*op & N_DEPEND) printf("%c", C_DEPEND);
}

void constraints_prettyPrint(constraint* cs) {
  printf("[");
  while (cs != NULL) {
    printf("\"");
    constraints_printOp(&cs->op);
    relation_prettyPrint(&cs->pkg);
    printf("\"");
    if(cs->next != NULL) {
      printf(",");
    }
    cs = cs->next;
  }
  printf("]\n");
}

void constraints_freeAll(constraint_list* cs) {
  constraint* c = cs->cons;
  while (c != NULL) {
    constraint* next = c->next;
    free(c->pkg.name);
    version_free(&c->pkg.version);
    free(c);
    c = next;
  }
  if (cs->json != NULL) {
    cJSON_Delete(cs->json);
  }
}

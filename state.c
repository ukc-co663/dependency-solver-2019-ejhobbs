#include "state.h"

states state_getFromFile(char *f) {
  states initialState = {0};
  char *fContents = getFullContents(f);

  cJSON *json = cJSON_Parse(fContents);
  if (json == NULL) {
    const char *e_ptr = cJSON_GetErrorPtr();
    if (e_ptr != NULL) {
      fprintf(stderr, "State: JSON parse error: %s\n", e_ptr);
    }
  } else {
    if (!cJSON_IsArray(json)) {
      fprintf(stderr, "Initial state expected array, got %#x!\n", json->type);
      cJSON_Delete(json);
    } else {
      int numItems = cJSON_GetArraySize(json);
      relation *allRelations = relation_getAll(json, numItems);
      state_member *sm = NULL;
      for (int i = 0; i < numItems; i++) {
        state_member *newSm = calloc(1, sizeof(*newSm));
        newSm->next = sm;
        newSm->rel = allRelations[i];
        sm = newSm;
      }
      initialState = (states){sm, json};
    }
  }
  free(fContents);
  return initialState;
}

int state_installed(states *s, char *name, version *v) {
  state_member *sm = s->members;
  while (sm != NULL) {
    relation thisRel = sm->rel;
    if (strcmp(thisRel.name, name) == 0 &&
        relation_compareVersion(&thisRel.version, v) == 0) {
      return 1;
    }
    sm = sm->next;
  }
  return 0;
}

void state_prettyPrint(states *s) {
  state_member *sm = s->members;
  printf("[");
  while (sm != NULL) {
    printf("\"");
    relation_prettyPrint(&sm->rel);
    printf("\"");
    if (sm->next != NULL) {
      printf(",");
    }
    sm = sm->next;
  }
  printf("]\n");
}

void state_freeAll(states *s) {
  state_member *sm = s->members;
  while (sm != NULL) {
    relation_free(&sm->rel);
    sm = sm->next;
    free(sm);
  }
  cJSON_Delete(s->json);
}

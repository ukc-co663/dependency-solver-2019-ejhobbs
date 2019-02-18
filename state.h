#ifndef SOLVE_STATE_H
#define SOLVE_STATE_H

#include "repository.h"
#include "file.h"

typedef struct state_member {
  struct state_member* prev;
  struct state_member* next;
  relation rel;
} state_member;

typedef struct states {
    state_member* members;
    cJSON* json;
} states;

int state_installed(states*, char*, version*);
states state_getFromFile(char*);
void state_freeAll(states*);
void state_prettyPrint(states*);

#endif //SOLVE_STATE_H

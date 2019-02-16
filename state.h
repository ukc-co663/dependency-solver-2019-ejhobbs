#ifndef SOLVE_STATE_H
#define SOLVE_STATE_H

#include "repository.h"
#include "file.h"

typedef struct states {
    int size;
    relation* members;
    cJSON* json;
} states;

states state_getFromFile(char*);
void state_freeAll(states*);
void state_prettyPrint(states*);

#endif //SOLVE_STATE_H

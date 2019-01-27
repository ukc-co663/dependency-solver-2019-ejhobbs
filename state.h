#ifndef SOLVE_STATE_H
#define SOLVE_STATE_H

#include "repository.h"
#include "file.h"

typedef struct states {
    int size;
    relation* members;
} states;

states state_getFromFile(char*);

#endif //SOLVE_STATE_H

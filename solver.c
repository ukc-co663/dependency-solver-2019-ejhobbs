#include "solver.h"
int installPackage(repository*, package*, states*, states*);
int uninstallPackage(repository*, package*, states*, states*);

int solver_newStateFromConstraints(repository* repo, constraints* instr, states* in, states* out){
  return 0;
}

int installPackage(repository* repo, package* pkg, states* in, states* out) {
  return 0;
}

int uninstallPackage(repository* repo, package* pkg, states* in, states* out){
  return 0;
}

constraints solver_toNewState(states* in, states* out){
  return (constraints){0, NULL, NULL};
}

#include "main.h"

int main(int argc, char** argv) {
  if (argc < _numargs) {
    fprintf(stderr, "Invalid number of arguments: %d \n\nUsage: \n solve repo initialState commands", argc-1);
    exit(1);
  }

  /* Get all packages from repository */
  repository repo = repo_getFromFile(argv[_repo]);
  if(repo.size <= 0) {
    fprintf(stderr, "No packages found while parsing repository, exiting\n");
    exit(1);
  }

  /* Get initial system state */
  states state = state_getFromFile(argv[_state]);
  constraints inputConstraints = constraints_getFromFile(argv[_constraints]);

  /* Find a state which satisfies all constraints */
  states newState;
  int result = solver_newStateFromConstraints(&repo, &inputConstraints, &state, &newState);

  if(!result) {
    fprintf(stderr, "Unable to satisfy given constraints, exiting\n");
    exit(1);
  }

  constraints outputConstraints = solver_toNewState(&state, &newState);

  constraints_freeAll(&outputConstraints);
  constraints_freeAll(&inputConstraints);
  state_freeAll(&state);
  repo_freeAll(&repo);
  return 0;
}

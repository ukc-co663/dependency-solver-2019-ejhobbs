#include "main.h"

int main(int argc, char** argv) {
  if (argc < _numargs) {
    fprintf(stderr, "Invalid number of arguments: %d \n\nUsage: \n solve repo initialState commands", argc-1);
    exit(1);
  }

  /* Get all packages from repository */
  repo_repository repo = repo_getFromFile(argv[_repo]);
  if(repo.size <= 0) {
    fprintf(stderr, "No packages found while parsing repository, exiting");
    exit(1);
  }

  /* Get initial system state */
  states state = state_getFromFile(argv[_state]);
  printf("Starting with %d packages already installed\n", state.size);

  constraints constraints = constraints_getFromFile(argv[_constraints]);
  printf("Given %d constraints to follow\n", constraints.size);

  state_prettyPrint(&state);

  constraints_freeAll(&constraints);
  state_freeAll(&state);
  repo_freeAll(&repo);
  return 0;
}

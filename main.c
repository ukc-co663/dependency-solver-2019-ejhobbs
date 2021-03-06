#include "main.h"

int main(int argc, char **argv) {
  if (argc < _numargs) {
    fprintf(stderr,
            "Invalid number of arguments: %d \n\nUsage: \n solve repo "
            "initialState commands",
            argc - 1);
    exit(1);
  }

  /* Get all packages from repository */
  repository repo = repo_getFromFile(argv[_repo]);
  if (repo.size <= 0) {
    fprintf(stderr, "No packages found while parsing repository, exiting\n");
    exit(1);
  }

  /* Get initial system state */
  states inputState = state_getFromFile(argv[_state]);
  constraint_list inputConstraints =
      constraints_getFromFile(argv[_constraints]);

  if (inputConstraints.cons == NULL) {
    /* no constraints, nothing to do! */
    printf("[]\n");
    exit(0);
  }

  option rules = solver_getRoute(&inputState, &repo, inputConstraints.cons);

  constraint *outputConstraints =
      solver_getConstraints(&repo, &inputState, &rules);
  constraints_prettyPrint(outputConstraints);

  constraints_freeAll(&inputConstraints);
  constraints_freeAll(&(constraint_list){outputConstraints, NULL});
  state_freeAll(&inputState);
  repo_freeAll(&repo);
  return 0;
}

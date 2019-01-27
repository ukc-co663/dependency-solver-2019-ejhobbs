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

  /* Print them all out */
  for (int i=0; i < repo.size; i++) {
    if(repo.packages[i] != NULL) {
      printf("%s\n", repo.packages[i]->name);
    }
  }
  repo_freeAll(repo);
  return 0;
}
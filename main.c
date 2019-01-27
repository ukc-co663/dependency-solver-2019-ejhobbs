#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cJSON.h>

#include "repository.h"
#include "main.h"

int main(int argc, char** argv) {
  if (argc < _numargs) {
    fprintf(stderr, "Invalid number of arguments: %d \n\nUsage: \n solve repo initialState commands", argc-1);
    exit(1);
  }
  char* input = getInput(argv[_repo]);

  cJSON *parsedJson = cJSON_Parse(input);

  if (parsedJson == NULL) {
    const char *e_ptr = cJSON_GetErrorPtr();
    if(e_ptr != NULL) {
      fprintf(stderr, "Repository: JSON parse error: %s\n", e_ptr);
    }
    end(input, parsedJson, 1);
  } else {
    /* Do some stuff */
    if (!cJSON_IsArray(parsedJson)){
      fprintf(stderr,"Repository expected array, got %#x!\n", parsedJson->type);
      end(input, parsedJson, 1);
    } else {
      package_group repo = repo_getAll(parsedJson);

      /* Print them all out */
      for (int i=0; i < repo.size; i++) {
        if(repo.packages[i] != NULL) {
          printf("%s\n", repo.packages[i]->name);
          //          package_prettyPrint(repo.packages[i]);
        }
      }
      repo_freeAll(repo);
    }
  }
  free(input);
  cJSON_Delete(parsedJson);
  return 0;
}

void end(char* input, cJSON* json, int status) {
  free(input);
  cJSON_Delete(json);
  exit(status);
}

char* getInput(const char* filename) {
  FILE* fp = fopen(filename, "r");

  if (fp != NULL) {
    fseek(fp, 0, SEEK_END);
    size_t len = (size_t) ftell(fp); /* find end */
    rewind(fp);
    char* input = malloc((len+1)*sizeof(char));
    fread(input, sizeof(char), len, fp);
    input[len] = 0; /* null terminate */
    fclose(fp);
    return input;
  }
  perror("Failed to open file");
  return NULL;
}

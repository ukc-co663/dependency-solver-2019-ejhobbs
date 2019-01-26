#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cJSON.h>

#include "repo_package.h"
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
    }

    int numPkgs = cJSON_GetArraySize(parsedJson);
    package** availablePkgs = malloc(numPkgs * sizeof *availablePkgs);

    /* Go from Json layout to array of struct*s */
    package** curPackage = availablePkgs;
    const cJSON *pkg = NULL;
    cJSON_ArrayForEach(pkg, parsedJson) {
      *curPackage = package_fromJson(pkg);
      curPackage++;
    }

    /* Deallocate everything */
    for (int i=0; i < numPkgs; i++) {
      if(availablePkgs[i] != NULL) {
//        package_prettyPrint(availablePkgs[i]);
        package_free(availablePkgs[i]);
      }
    }
    free(availablePkgs);
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
    long len = ftell(fp); /* find end */
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

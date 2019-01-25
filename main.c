#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cJSON.h>

#include "main.h"

int main(int argc, char** argv) {
  if (argc < _numargs) {
    fprintf(stderr, "Invalid number of arguments: %d \n\nUsage: \n solve repo initialState commands", argc-1);
    return 1;
  }
  char* input = getInput(argv[_repo]);

  cJSON *parsedJson = cJSON_Parse(input);

  if (parsedJson == NULL) {
    const char *e_ptr = cJSON_GetErrorPtr();
    if(e_ptr != NULL) {
      fprintf(stderr, "JSON parse error: %s\n", e_ptr);
    }
    free(input);
    return 1;
  } else {
    /* Do some stuff */
    if (!cJSON_IsArray(parsedJson)){
      fprintf(stderr,"Expected array, got %#x!\n", parsedJson->type);
      return 1;
    }

    int numPkgs = cJSON_GetArraySize(parsedJson);
    package** availablePkgs = malloc(numPkgs * sizeof *availablePkgs);
    /* Go from Json layout to array of struct*s */
    for (int i=0; i < numPkgs; i++) {
      availablePkgs[i] = packageFromJson(cJSON_GetArrayItem(parsedJson, i));
    }
    /* Deallocate everything */
    for (int i=0; i < numPkgs; i++) {
      if(availablePkgs[i] != NULL) {
        printf("%d name: %s, size: %d\n",i, availablePkgs[i]->name, availablePkgs[i]->size);
        free(availablePkgs[i]);
      }
    }
    free(availablePkgs);
  }
  free(input);
  return 0;
}

char* getInput(char* filename) {
  char* input = NULL;
  char line[1024];
  int size = 0;

  FILE* fp = fopen(filename, "r");

  if (fp != NULL) {
    while (fscanf(fp, "%s[^\n]", line) != EOF) {
      int lineSize = strlen(line);
      input = realloc(input, size + lineSize + 1);
      memcpy(input + size, line, lineSize);
      size += lineSize;
    }
    return input;
  }
  perror("Failed to open file");
  return NULL;
}

package* packageFromJson(cJSON* jsonPkg){
  cJSON* name = cJSON_GetObjectItemCaseSensitive(jsonPkg, "name");
  cJSON* size = cJSON_GetObjectItemCaseSensitive(jsonPkg, "size");

  if (!cJSON_IsNumber(size) || !cJSON_IsString(name)) {
    fprintf(stderr, "Malformed package information\n");
    return NULL;
  }

  package* parsed = malloc(sizeof *parsed);

  parsed->name = name->valuestring;
  parsed->size = size->valueint;
  return parsed;
}

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cJSON.h>

#include "main.h"

int main(void) {
  char* input = getInput();

  cJSON *parsedJson = cJSON_Parse(input);

  if (parsedJson == NULL) {
    const char *e_ptr = cJSON_GetErrorPtr();
    if(e_ptr != NULL) {
      fprintf(stderr, "Error: %s\n", e_ptr);
    }
    free(input);
    return 1;
  } else {
    /* Do some stuff */
    if (!cJSON_IsArray(parsedJson)){
      perror("Expected array, got not that!\n");
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
      printf("{name: %s, size: %d}\n", availablePkgs[i]->name, availablePkgs[i]->size);
      if(availablePkgs[i] != NULL) {
        free(availablePkgs[i]);
      }
    }
    free(availablePkgs);
  }
  free(input);
  return 0;
}

char* getInput(void) {
  char* input = NULL;
  char line[4096];
  int size = 0;
  while (scanf("%s[^\n]", line) != EOF) {
    int lineSize = strlen(line);
    input = realloc(input, size + lineSize + 1);
    memcpy(input + size, line, lineSize);
    size += lineSize;
  }
  return input;
}

package* packageFromJson(cJSON* jsonPkg){
  cJSON* name = cJSON_GetObjectItemCaseSensitive(jsonPkg, "name");
  cJSON* size = cJSON_GetObjectItemCaseSensitive(jsonPkg, "size");

  if (!cJSON_IsNumber(size) || !cJSON_IsString(name)) {
    fprintf(stderr, "Malformed package information");
    return NULL;
  }

  package* parsed = malloc(sizeof *parsed);

  parsed->name = name->valuestring;
  parsed->size = size->valueint;
  return parsed;
}

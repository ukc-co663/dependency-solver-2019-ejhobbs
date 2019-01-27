#include <stdio.h>
#include <stdlib.h>
#include "file.h"

char* getFullContents(const char *filename) {
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

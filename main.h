typedef struct package {
  char* name;
  int size;
} package;

int main(void);
package* packageFromJson(cJSON*);
char* getInput();

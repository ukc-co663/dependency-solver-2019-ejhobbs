#define _repo 1
#define _state 2
#define _commands 3
#define _numargs 2

typedef struct package {
  char* name;
  int size;
} package;

int main(int, char**);
package* packageFromJson(cJSON*);
char* getInput(char*);

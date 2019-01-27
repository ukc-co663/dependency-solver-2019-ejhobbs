#include "state.h"

states state_getFromFile(char* f) {
    states initialState = {0};
    char* fContents = getFullContents(f);

    cJSON* json = cJSON_Parse(fContents);
    if(json == NULL) {
        const char *e_ptr = cJSON_GetErrorPtr();
        if(e_ptr != NULL) {
            fprintf(stderr, "Repository: JSON parse error: %s\n", e_ptr);
        }
    } else {
        if (!cJSON_IsArray(json)){
            fprintf(stderr, "Initial state expected array, got %#x!\n", json->type);
        } else {
            int numItems = cJSON_GetArraySize(json);
            relation* allRelations = relation_getAll(json, numItems);
            initialState = (states){numItems, allRelations};
        }
    }
    return initialState;
}

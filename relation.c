#include "relation.h"

relation* relation_getAll(const cJSON *relationList, int count) {
    relation* relations = NULL;
    if(count > 0) {
        relations = calloc((size_t) count, sizeof(relation));
        cJSON* rel = NULL;
        int i = 0;
        cJSON_ArrayForEach(rel, relationList) {
            relations[i] = parseRelation(rel->valuestring);
            i++;
        }
    }
    return relations;
}

relation parseRelation(char* rel) {
    int sName = 0; /* Size of pkg name */
    int comp = 0; /* Start with any */
    int sym = 0;
    char* curChar = rel;
    /* Loop over the string until we get to the first symbol, ie end of the name*/
    while(sName < strlen(rel) && !sym){
        switch(*curChar) {
            case '<':
            case '>':
            case '=':
                sym = 1;
                break;
            default:
                sName++;
                break;
        }
        if(!sym){
            curChar++;
        }
    }
    char* name = calloc((size_t) sName+1, sizeof(char));
    strncpy(name, rel, sName);
    name[sName] = 0;
    int num = 0;
    while(curChar < rel + strlen(rel) && !num) {
        switch(*curChar) {
            case '<':
                comp |= _lt;
                break;
            case '>':
                comp |= _gt;
                break;
            case '=':
                comp |= _eq;
                break;
            default:
                num=1;
                break;
        }
        if (!num) {
            curChar++;
        }
    }
    version ver = {0, NULL};
    if(num) {
        ver = parseVersion(curChar);
    }

    relation newRel = {name, ver, comp};
    return newRel;
}

version parseVersion(char *versionString) {
    int* parts = NULL;
    int vSize = 0;
    char delim[] = ".";
    char* part = strtok(versionString, delim);
    if(part == NULL) {
        parts = malloc(sizeof(int));
        vSize = 1;
        *parts = atoi(versionString);
    } else {
        while (part != NULL) {
            int* tmp = realloc(parts, (vSize+1)*sizeof(int));
            if(tmp != NULL) {
                parts = tmp;
                parts[vSize] = atoi(part);
                part = strtok(NULL, delim);
                vSize++;
            } else {
                perror("Something went wrong");
            }
        }
    }
    version vers = {vSize, parts};
    return vers;
}

void relation_free(int s, relation* r) {
    for (int i = 0; i < s; i++) {
        free(r[i].name);
        version_free(&(r[i].version));
    }
    free(r);
}

void version_free(version* v) {
    if (v->size > 0) {
        free(v->val);
    }
}
#include "src/vector.h"
#include "src/std__map.h"


#include <assert.h>

init_std__map(char, int)

int int_cmp(char *a, char *b) {
    return *(char *)a - *(char *)b;
}

int main() {
    std__map(char, int) *node = new_std__map(char, int)(int_cmp);

    char *keys[10];
    int *values[10];

    for (int i = 0; i < 10; i++) {
        char *k = malloc(sizeof(char));
        int *v = malloc(sizeof(int));
        *k = 65 + i;
        *v = i * i;
        node->insert(node, k, v);
        keys[i] = k;
        values[i] = v;
    }

    char *k = malloc(sizeof(char));
    *k = 65+5;
    int *v = node->get(node, k);
    if (v == NULL) {
        printf("Key not found\n");
    } else {
        printf("Node with key %c value: %d\n", *k, *v);
    }
    free(k);

    delete_std__map(char, int)(node);

    for (int i = 0; i < 10; i++) {
        free(keys[i]);
        free(values[i]);
    }

    return 0;
}

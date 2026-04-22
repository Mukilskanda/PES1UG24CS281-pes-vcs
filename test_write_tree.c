#include "tree.h"
#include "pes.h"
#include <stdio.h>

int main() {
    ObjectID id;

    if (tree_from_index(&id) < 0) {
        printf("Tree creation failed\n");
        return 1;
    }

    printf("Tree object created!\n");
    return 0;
}

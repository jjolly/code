#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include"btree.h"

int btree_test_compare(uintptr_t n1, uintptr_t n2) {
    return((n1 < n2) ? -1 : ((n1 > n2) ? 1 : 0));
}

void btree_test_dump(FILE *fp, uintptr_t data) {
    if ( data == 0 ) fprintf(fp, ".");
    else fprintf(fp, "%d", data);
}

void btree_test_free(uintptr_t key, uintptr_t value) {
    return;
}

BTREE_DEFINE(test, uintptr_t, uintptr_t)

#define MAX_COUNT (9)

int main(int argc, char *argv[]) {
    btree bt;

    btree_test_init(&bt, 5);

    int i;
    for ( i = 1; i <= MAX_COUNT * 111; i++ ) {
        btree_test_insert(&bt, i, 0);
    }
    btree_dump(stderr, &bt);
    for ( i = MAX_COUNT; i > 0; i-- ) {
        btree_test_remove(&bt, (i*7)%10);
        btree_dump(stderr, &bt);
    }

    btree_free(&bt);
}

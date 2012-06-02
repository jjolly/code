#include<stdio.h>

typedef struct btree_t {
    void *root;
    int order;
    int (*key_compare)(void *, void *);
    void (*key_dump)(FILE *, void *);
    void (*key_free)(void *, void *);
} btree;

void btree_init(btree *bt, int order);

void btree_free(btree *bt);

void btree_insert(btree *bt, void *k, void *v);

void btree_remove(btree *bt, void *k);

void *btree_find(btree *bt, void *k);

void btree_dump(FILE *out, btree *bt);

#define BTREE_DEFINE(name, keytype, valtype) \
    int btree_##name##_compare_cast(void *k1, void *k2) { \
        return btree_##name##_compare((keytype)k1, (keytype)k2); \
    } \
\
    void btree_##name##_dump_cast(FILE *fp, void *k) { \
        btree_##name##_dump(fp, (keytype)k); \
    } \
\
    void btree_##name##_free_cast(void *k, void *v) { \
        btree_##name##_free((keytype)k, (valtype)v);\
    } \
\
    void btree_##name##_init(btree *bt, int order) { \
        bt->key_compare = btree_##name##_compare_cast; \
        bt->key_dump = btree_##name##_dump_cast; \
        bt->key_free = btree_##name##_free_cast; \
        btree_init(bt, order); \
    } \
\
    void btree_##name##_insert(btree *bt, keytype k, valtype v) { \
        btree_insert(bt, (void *)k, (void *)v); \
    } \
\
    valtype btree_##name##_find(btree *bt, keytype k) { \
        return (valtype)btree_find(bt, (void *)k); \
    } \
\
    void btree_##name##_remove(btree *bt, keytype k) { \
        btree_remove(bt, (void *)k); \
    }

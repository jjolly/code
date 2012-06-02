#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include"btree.h"

#define BTREE_NODES(o) ((o) * 2)
#define BTREE_CHILDREN(o) (BTREE_NODES(o) + 1)

#define btree_set_root(bt, rt) (((bt)->root) = ((void *)(rt)))
#define btree_get_root(bt, rt) ((rt) = ((btree_node *)((bt)->root)))

typedef struct btree_node_t {
    btree *bt;
    void **keys;
    void **values;
    struct btree_node_t **child;
} btree_node;

void btree_init(btree *bt, int order) {
    assert(bt->key_compare);
    assert(bt->key_dump);
    assert(bt->key_free);
    btree_set_root(bt, NULL);
    bt->order = order;
}

void free_node(btree_node *n) {
    int i;
    if ( n == NULL ) return;
    for ( i = 0; i < BTREE_NODES(n->bt->order); i++ ) {
        if ( n->keys[i] != NULL ) n->bt->key_free(n->keys[i], n->values[i]);
        free_node(n->child[i]);
    }
    free_node(n->child[i]);
    free(n->keys);
    free(n->values);
    free(n->child);
    free(n);
}

void btree_free(btree *bt) {
    btree_node *r;
    btree_get_root(bt, r);
    if ( r != NULL ) free_node(r);
    btree_set_root(bt, NULL);
}

btree_node *allocate_node(btree *bt) {
    int i;
    btree_node *n = malloc(sizeof(btree_node));
    n->keys = malloc(sizeof(void *) * BTREE_NODES(bt->order));
    n->values = malloc(sizeof(void *) * BTREE_NODES(bt->order));
    n->child = malloc(sizeof(btree_node *) * BTREE_CHILDREN(bt->order));
    n->bt = bt;
    n->child[0] = NULL;
    for ( i = BTREE_NODES(bt->order); i > 0; i-- ) {
        n->child[i] = NULL;
        n->keys[i-1] = NULL;
        n->values[i-1] = NULL;
    }
    return n;
}

btree_node *btree_add_to_node(btree *bt, btree_node *r, void *k, void *v) {
    int i, j, dif;
    btree_node *savenode;
    void *savekey, *saveval;

    if ( r == NULL ) {
        r = allocate_node(bt);
        r->keys[0] = k;
        r->values[0] = v;
    } else {
        for ( i = 0; i < BTREE_NODES(bt->order) && r->keys[i] != NULL && (dif = bt->key_compare(k, r->keys[i])) > 0; i++ );
        if ( dif != 0 ) {
            /* No match found, lets add this node to the left child */
            btree_node *n = btree_add_to_node(bt, r->child[i], k, v);
            if ( r->child[i] != n ) {
                j = BTREE_NODES(bt->order) - 1;
                savekey = r->keys[j];
                saveval = r->values[j];
                savenode = r->child[j+1];

                /* New node has been created. Add key and child pointers */
                /* First shift node on the right over one */
                for ( ; j > i; j-- ) {
                    r->keys[j] = r->keys[j-1];
                    r->values[j] = r->values[j-1];
                    r->child[j+1] = r->child[j];
                }

                /* Copy the data from the new node */
                if ( i == BTREE_NODES(bt->order) ) {
                    savekey = n->keys[0];
                    saveval = n->values[0];
                    savenode = n->child[1];
                } else {
                    r->keys[i] = n->keys[0];
                    r->values[i] = n->values[0];
                    r->child[i+1] = n->child[1];
                }
                r->child[i] = n->child[0];

                n->keys[0] = NULL;
                n->values[0] = NULL;
                n->child[0] = NULL;
                n->child[1] = NULL;

                if ( savekey != NULL ) {
                    /* Splitsville, baby! */
                    btree_node *newroot = allocate_node(r->bt);

                    /* Move the median key to the new root */
                    newroot->keys[0] = r->keys[bt->order];
                    newroot->values[0] = r->values[bt->order];
                    r->keys[bt->order] = NULL;
                    r->values[bt->order] = NULL;
                    /* ... and make the old root and new node children */
                    newroot->child[0] = r;
                    newroot->child[1] = n;

                    /* Copy the node right of the median into the new node */
                    for ( i = 0; i < bt->order - 1; i++ ) {
                        n->keys[i] = r->keys[i + bt->order + 1];
                        r->keys[i + bt->order + 1] = NULL;
                        n->values[i] = r->values[i + bt->order + 1];
                        r->values[i + bt->order + 1] = NULL;
                        n->child[i] = r->child[i + bt->order + 1];
                        r->child[i + bt->order + 1] = NULL;
                    }
                    n->child[i] = r->child[i + bt->order + 1];
                    r->child[i + bt->order + 1] = NULL;
                    n->keys[i] = savekey;
                    n->values[i] = savekey;
                    n->child[i + 1] = savenode;

                    r = newroot;
                } else {
                    /* Don't need that node anymore */
                    free_node(n);
                }
            }
        }
    }

    return r;
}

void btree_insert(btree *bt, void *k, void *v) {
    btree_node *r;
    btree_get_root(bt, r);

    btree_set_root(bt, btree_add_to_node(bt, r, k, v));
}

void *btree_find_node(btree_node *r, void *k) {
    if (r == NULL) return NULL;
    int i;

    for ( i = 0; i < BTREE_NODES(r->bt->order); i++ ) {
        int diff = r->bt->key_compare(k, r->keys[i]);
        if ( diff == 0 ) return r->values[i];
        else if ( diff == -1 ) break;
    }
    return btree_find_node(r->child[i], k);
}

void *btree_find(btree *bt, void *k) {
    btree_node *r;
    btree_get_root(bt, r);

    return btree_find_node(r, k);
}

int count_keys(btree_node *r) {
    int i;
    if ( r == NULL ) return 0;
    for ( i = 0; i < BTREE_NODES(r->bt->order) && r->keys[i] != NULL; i++ );
    return i;
}

btree_node *btree_remove_node(btree_node *r, void *k) {
    if (r == NULL ) return;
    int i, j, rightcount;

    for ( i = 0; i < BTREE_NODES(r->bt->order) && r->keys[i] != NULL; i++ ) {
        int diff = r->bt->key_compare(k, r->keys[i]);
        if ( diff == 1 ) continue;
        if ( diff == 0 ) {
            if ( r->child[i + 1] == NULL ) {
                /* Remove the key and value */
                r->bt->key_free(r->keys[i], r->values[i]);
                /* Shift everything left */
                for ( j = i; j < BTREE_NODES(r->bt->order) - 1 && r->keys[j+1] != NULL; j++ ) {
                    r->keys[j] = r->keys[j+1];
                    r->values[j] = r->values[j+1];
                    /* The child links should all be null, no reason to move */
                }
                r->keys[j] = NULL;
                r->values[j] = NULL;
            } else {
                /* We're not in a leaf node. Find the leaf and swap */
                void *tmpkey = r->keys[i];
                void *tmpval = r->values[i];
                btree_node *leaf = r->child[i + 1];
                while ( leaf->child[0] != NULL ) leaf = leaf->child[0];
                r->keys[i] = leaf->keys[0];
                r->values[i] = leaf->values[0];
                leaf->keys[0] = tmpkey;
                leaf->values[0] = tmpval;
                /* Bump to the next node so we follow the correct child */
                i++;
            }
        }
        break;
    }
    btree_remove_node(r->child[i], k);

    /* Count the keys in the child node to determine if a balance is needed */
    j = count_keys(r->child[i]);
    if ( r->child[i] != NULL && j < r->child[i]->bt->order ) {
        /* We need at least one child to the left */
        if ( i == 0 ) i++;
        int rightkey = 0;
        int keycount = count_keys(r->child[i-1]);
        keycount += count_keys(r->child[i]) + 1;
        if ( i < BTREE_NODES(r->bt->order) && r->child[i+1] != NULL ) {
            rightkey = 1;
            keycount += count_keys(r->child[i+1]) + 1;
        }
        void **keyarray = malloc(sizeof(void *) * keycount);
        void **valarray = malloc(sizeof(void *) * keycount);
        btree_node **childarray = malloc(sizeof(btree_node *) * (keycount + 1));
        childarray[0] = r->child[i-1]->child[0];
        r->child[i-1]->child[0] = NULL;
        for ( j = 0; j < BTREE_NODES(r->child[i-1]->bt->order) && r->child[i-1]->keys[j] != NULL; j++ ) {
            keyarray[j] = r->child[i-1]->keys[j];
            valarray[j] = r->child[i-1]->values[j];
            childarray[j+1] = r->child[i-1]->child[j+1];
            r->child[i-1]->keys[j] = NULL;
            r->child[i-1]->values[j] = NULL;
            r->child[i-1]->child[j+1] = NULL;
        }
        keyarray[j] = r->keys[i-1];
        valarray[j] = r->values[i-1];
        r->keys[i-1] = NULL;
        r->values[i-1] = NULL;
        keycount = j + 1;
        childarray[keycount] = r->child[i]->child[0];
        r->child[i]->child[0] = NULL;
        for ( j = 0; j < BTREE_NODES(r->child[i]->bt->order) && r->child[i]->keys[j] != NULL; j++ ) {
            keyarray[j + keycount] = r->child[i]->keys[j];
            valarray[j + keycount] = r->child[i]->values[j];
            childarray[j + 1 + keycount] = r->child[i]->child[j+1];
            r->child[i]->keys[j] = NULL;
            r->child[i]->values[j] = NULL;
            r->child[i]->child[j+1] = NULL;
        }
        keycount += j;
        if ( rightkey ) {
            keyarray[keycount] = r->keys[i];
            valarray[keycount] = r->values[i];
            r->keys[i] = NULL;
            r->values[i] = NULL;
            keycount++;
            childarray[keycount] = r->child[i+1]->child[0];
            r->child[i+1]->child[0] = NULL;
            for ( j = 0; j < BTREE_NODES(r->child[i+1]->bt->order) && r->child[i+1]->keys[j] != NULL; j++ ) {
                keyarray[j + keycount] = r->child[i+1]->keys[j];
                valarray[j + keycount] = r->child[i+1]->values[j];
                childarray[j + 1 + keycount] = r->child[i+1]->child[j+1];
                r->child[i+1]->keys[j] = NULL;
                r->child[i+1]->values[j] = NULL;
                r->child[i+1]->child[j+1] = NULL;
            }
            keycount += j;

            /* Since we're dealing with the right key, check if it needs to
             * be merged out or if it can stick around */
            if ( keycount < r->bt->order * 3 + 2 ) {
                /* There are not enough keys to support this structure. Merge */
                free_node(r->child[i+1]);
                for ( j = i; j < BTREE_NODES(r->bt->order) - 1 && r->keys[j+1] != NULL; j++ ) {
                    r->keys[j] = r->keys[j+1];
                    r->values[j] = r->values[j+1];
                    r->child[j+1] = r->child[j+2];
                }
                r->keys[j] = NULL;
                r->values[j] = NULL;
                r->child[j+1] = NULL;
            } else {
                /* Take our fair share and let the rest be shared between the
                 * two children to the left */
                rightcount = (keycount - 2) / 3;
                keycount -= rightcount + 1;
                r->keys[i] = keyarray[keycount];
                r->values[i] = valarray[keycount];
                r->child[i+1]->child[0] = childarray[keycount + 1];
                for ( j = 0; j < rightcount; j++ ) {
                    r->child[i+1]->keys[j] = keyarray[keycount + j + 1];
                    r->child[i+1]->values[j] = keyarray[keycount + j + 1];
                    r->child[i+1]->child[j+1] = childarray[keycount + j + 2];
                }
            }
        }

        /* Are there enough keys to spread between two children? */
        if ( keycount <= BTREE_NODES(r->bt->order) ) {
            /* Nope, delete the key, shift the other keys */
            free_node(r->child[i]);
            for ( j = i - 1; j < BTREE_NODES(r->bt->order) - 1 && r->keys[j+1] != NULL; j++) {
                r->keys[j] = r->keys[j+1];
                r->values[j] = r->values[j+1];
                r->child[j+1] = r->values[j+2];
            }
            r->keys[j] = NULL;
            r->values[j] = NULL;
            r->child[j+1] = NULL;
        } else {
            rightcount = (keycount - 1) / 2;
            keycount -= rightcount + 1;
            r->keys[i-1] = keyarray[keycount];
            r->values[i-1] = valarray[keycount];
            r->child[i]->child[0] = childarray[keycount + 1];
            for ( j = 0; j < rightcount; j++ ) {
                r->child[i]->keys[j] = keyarray[keycount + j + 1];
                r->child[i]->values[j] = keyarray[keycount + j + 1];
                r->child[i]->child[j+1] = childarray[keycount + j + 2];
            }
        }

        r->child[i-1]->child[0] = childarray[0];
        for ( j = 0; j < keycount; j++ ) {
            r->child[i-1]->keys[j] = keyarray[j];
            r->child[i-1]->values[j] = valarray[j];
            r->child[i-1]->child[j+1] = childarray[j+1];
        }
        free(keyarray);
        free(valarray);
        free(childarray);
    }
}

void btree_remove(btree *bt, void *k) {
    btree_node *r;
    btree_get_root(bt, r);

    btree_remove_node(r, k);

    /* When the root node is out of keys, delete the root node */
    if ( r->keys[0] == NULL ) {
        btree_node *tmpnode = r->child[0];
        r->child[0] = NULL;
        free_node(r);
        r = tmpnode;
    }

    btree_set_root(bt, r);
}

btree_dump_node(FILE *out, btree_node *r, const char *prefix) {
    if ( r == NULL ) return;
    int i;
    int prefix_len = strlen(prefix);
    char *new_prefix = malloc(prefix_len + 3);

    strcpy(new_prefix, prefix);
    strcat(new_prefix, "| ");

    fprintf(out, "[");
    for ( i = 0; i < BTREE_NODES(r->bt->order); i++ ) {
        r->bt->key_dump(out, r->keys[i]);
        if ( i < BTREE_NODES(r->bt->order) - 1 ) fprintf(out, ",");
    }
    fprintf(out, "]\n");

    for ( i = 0; i < BTREE_CHILDREN(r->bt->order); i++ ) {
        if ( r->child[i] != NULL ) fprintf(out, "%s+-", prefix);
        if ( i == BTREE_CHILDREN(r->bt->order) - 1 || r->child[i+1] == NULL ) new_prefix[prefix_len] = ' ';
        btree_dump_node(out, r->child[i], new_prefix);
    }

    free(new_prefix);
}

void btree_dump(FILE *out, btree *bt) {
    btree_node *r;
    btree_get_root(bt, r);
    btree_dump_node(out, r, "");
}

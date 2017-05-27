#include<stdio.h>
#include<stdlib.h>

#define RBT_RED (0)
#define RBT_BLK (1)

typedef struct rbnode_t {
	struct rbnode_t *parent;
	struct rbnode_t *left;
	struct rbnode_t *right;
	int rb;
	int key;
} rbnode;

typedef struct rbtree_t {
	rbnode *head;
} rbtree;

rbtree *rbt_alloc() {
	rbtree *rbt = malloc(sizeof(rbtree));
	rbt->head = NULL;
}

void rbt_insrec(rbnode **p, rbnode *c) {
	if(*p == NULL) *p = c;
	else {
		rbnode **next = NULL;

		if((*p)->key > c->key) next = &(*p)->left;
		else next = &(*p)->right;

		if(*next == NULL) {
			*next = c;
			c->parent = *p;
		} else rbt_insrec(next, c);
	}
}

rbnode *rbt_gp(rbnode *n) {
	if(n != NULL && n->parent != NULL) return n->parent->parent;
	else return NULL;
}

rbnode *rbt_ps(rbnode *n) {
	rbnode *g = rbt_gp(n);
	if(g != NULL) {
		if(g->left == n->parent) g = g->right;
		else g = g->left;
	}
	return g;
}

void rbt_rotl(rbnode *n) {
	rbnode *g = n->parent, *p = n->right, *c = n->right->left;
	// Fix new parent
	if(g != NULL) {
		if(g->left == n) g->left = p;
		else g->right = p;
	}
	p->parent = g;
	// Fix new child
	n->right = c;
	if(c != NULL) c->parent = n;
	// Fix current node
	p->left = n;
	n->parent = p;
}

void rbt_rotr(rbnode *n) {
	rbnode *g = n->parent, *p = n->left, *c = n->left->right;
	// Fix new parent
	if(g != NULL) {
		if(g->left == n) g->left = p;
		else g->right = p;
	}
	p->parent = g;
	// Fix new child
	n->left = c;
	if(c != NULL) c->parent = n;
	// Fix current node
	p->right = n;
	n->parent = p;
}

void rbt_bal(rbnode *n) {
	// If the head node is red, turn it black and quit.
	if(n->parent == NULL) n->rb = RBT_BLK;
	// We only need to do work if the parent node is red.
	else if(n->parent->rb == RBT_RED) {
		rbnode *g = rbt_gp(n), *u = rbt_ps(n);
		// Is the parent's sibling red also?
		// Then convert both to black, the grandparent to red,
		// and check if the grandparent needs balancing.
		if(u != NULL && u->rb == RBT_RED) {
			n->parent->rb = RBT_BLK;
			u->rb = RBT_BLK;
			g->rb = RBT_RED;
			rbt_bal(g);
		} else {
			// In order to push the parent's sibling down, we need
			// to make the tree right- or left-leaning.
			if(n == n->parent->right && n->parent == g->left) {
				// This pushes anything needing to be left to the left.
				rbt_rotl(n->parent);
				n = n->left;
			} else if(n == n->parent->left && n->parent == g->right) {
				// This pushes anything needing to be right to the right.
				rbt_rotr(n->parent);
				n = n->right;
			}
			// The tree is either right- or left- leaning.
			// This pushes the parent's black sibling down and makes
			// the grandparent a child of my parent.
			g = rbt_gp(n);
			n->parent->rb = RBT_BLK;
			g->rb = RBT_RED;
			if(n == n->parent->left) rbt_rotr(g);
			else rbt_rotl(g);
		}
	}
}

void rbt_ins(rbtree *tree, int key) {
	rbnode *newnode = malloc(sizeof(rbnode));
	newnode->parent = NULL;
	newnode->left = NULL;
	newnode->right = NULL;
	newnode->rb = RBT_RED;
	newnode->key = key;

	// Simple unbalanced binary tree add
	rbt_insrec(&tree->head, newnode);

	// The actual balancing
	rbt_bal(newnode);

	// There's a chance the old head was rotated. Find the new head.
	while(tree->head->parent != NULL) tree->head = tree->head->parent;
}

int rbt_inorder(rbnode *node) {
	int depth = 1;
	if(node != NULL) {
		printf("(");
		int ldepth = rbt_inorder(node->left);
		printf("%c%d", node->rb == RBT_RED ? 'R' : 'B', node->key);
		int rdepth = rbt_inorder(node->right);
		printf(")");
		if(ldepth != rdepth) {
			printf("\nAt node %d, ldepth:%d, rdepth:%d\n", node->key, ldepth, rdepth);
		}
		depth = (node->rb == RBT_BLK ? 1 : 0) + (ldepth > rdepth ? ldepth : rdepth);
	}
	return depth;
}

int main() {
	int num;
	rbtree *tree = rbt_alloc();

	while(fscanf(stdin, "%d", &num) == 1)
		rbt_ins(tree, num);

	int depth = rbt_inorder(tree->head);
	printf("\nDepth:%d\n", depth);
}

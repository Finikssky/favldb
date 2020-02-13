#include <stdio.h>
#include <stdlib.h>

#include "avl_tree.h"

#include "pool_allocator.h"

static int height(const avl_node_t *an)
{
	return an != NULL ? an->height : 0;
}

static void recalc_height(avl_node_t *an)
{
	int hl = height(an->left);
	int hr = height(an->right);

	an->height = 1 + ((hl > hr) ? hl : hr);
}

static void rotate_left(avl_node_t **root)
{
	avl_node_t *b = *root;
	avl_node_t *d = b->right;
	avl_node_t *c;

	c = d->left;
	b->right = c;

	if (c != NULL)
		c->parent = b;

	recalc_height(b);

	d->left = b;
	d->parent = b->parent;
	b->parent = d;
	recalc_height(d);

	*root = d;
}

static void rotate_right(avl_node_t **root)
{
	avl_node_t *d = *root;
	avl_node_t *b = d->left;
	avl_node_t *c;

	c = b->right;
	d->left = c;

	if (c != NULL)
		c->parent = d;

	recalc_height(d);

	b->right = d;
	b->parent = d->parent;
	d->parent = b;
	recalc_height(b);

	*root = b;
}

static void rotate_left_right(avl_node_t **root)
{
	avl_node_t *f = *root;
	avl_node_t *b = f->left;
	avl_node_t *d = b->right;
	avl_node_t *c;
	avl_node_t *e;

	c = d->left;
	b->right = c;

	if (c != NULL)
		c->parent = b;

	recalc_height(b);

	e = d->right;
	f->left = e;

	if (e != NULL)
		e->parent = f;

	recalc_height(f);

	d->left = b;
	d->right = f;
	d->parent = f->parent;
	b->parent = d;
	f->parent = d;
	recalc_height(d);

	*root = d;
}

static void rotate_right_left(avl_node_t **root)
{
	avl_node_t *b = *root;
	avl_node_t *f = b->right;
	avl_node_t *d = f->left;
	avl_node_t *c;
	avl_node_t *e;

	c = d->left;
	b->right = c;

	if (c != NULL)
		c->parent = b;

	recalc_height(b);

	e = d->right;
	f->left = e;

	if (e != NULL)
		e->parent = f;

	recalc_height(f);

	d->left = b;
	d->right = f;
	d->parent = b->parent;
	b->parent = d;
	f->parent = d;
	recalc_height(d);

	*root = d;
}

static int balance(const avl_node_t *an)
{
	return height(an->right) - height(an->left);
}

static void rebalance_node(avl_node_t **_root)
{
	avl_node_t *root = *_root;
	int bal = balance(root);
	if (bal == -2)
	{
		if (balance(root->left) <= 0)
			rotate_right(_root);
		else
			rotate_left_right(_root);
	}
	else if (bal == 2)
	{
		if (balance(root->right) < 0)
			rotate_right_left(_root);
		else
			rotate_left(_root);
	}
}

static avl_node_t **find_reference(avl_tree_t *tree, const avl_node_t *an)
{
	if (an->parent != NULL)
	{
		if (an->parent->left == an)
			return &an->parent->left;
		else
			return &an->parent->right;
	}
	else
	{
		return &tree->root;
	}
}

static void replace_reference(avl_tree_t *tree,
			      const avl_node_t *an,
			      avl_node_t *new_child)
{
	*find_reference(tree, an) = new_child;
}

static void rebalance_path(avl_tree_t *tree, avl_node_t *an)
{
	while (an != NULL)
	{
		int old_height = an->height;
		recalc_height(an);

		avl_node_t **ref = find_reference(tree, an);
		rebalance_node(ref);
		an = *ref;

		if (old_height == an->height)
			break;

		an = an->parent;
	}
}

int avl_tree_insert(avl_tree_t *tree, void *data)
{
	if (!tree)
		return -2;

	if (tree->print)
		tree->print(data, "insert");

	avl_node_t *an = p_alloc(tree->allocator);
	if (!an)
	{
		return -2;
	}

	/*
	 * Find the node to which an is to be attached as a leaf.
	 */
	avl_node_t *p = NULL;
	avl_node_t **pp = &tree->root;

	while (*pp != NULL)
	{
		int ret;

		p = *pp;

		ret = tree->compare(data, p->data);
		if (ret < 0)
			pp = &p->left;
		else if (ret > 0)
			pp = &p->right;
		else
			return -1;
	}

	/*
	 * Insert an.
	 */



	an->left = NULL;
	an->right = NULL;
	an->parent = p;
	an->height = 1;
	an->data = data;
	*pp = an;

	/*
	 * Start rebalancing from an's parent.
	 */
	rebalance_path(tree, p);

	tree->elements++;

	return 0;
}

static avl_node_t * avl_tree_delete_leaf(avl_tree_t *tree, avl_node_t *an)
{
	/*
	 * Simply replace the reference from an's parent to an by NULL,
	 * and start rebalancing from an's parent.
	 */
	replace_reference(tree, an, NULL);

	return an->parent;
}

static avl_node_t * avl_tree_delete_nonleaf(avl_tree_t *tree, avl_node_t *an)
{
	avl_node_t *victim;
	avl_node_t *p;

	if (height(an->left) > height(an->right))
	{
		victim = an->left;
		while (victim->right != NULL)
			victim = victim->right;

		replace_reference(tree, victim, victim->left);
		if (victim->left != NULL)
			victim->left->parent = victim->parent;
	}
	else
	{
		victim = an->right;
		while (victim->left != NULL)
			victim = victim->left;

		replace_reference(tree, victim, victim->right);
		if (victim->right != NULL)
			victim->right->parent = victim->parent;
	}

	/*
	 * We will start rebalancing the tree from the victim node's
	 * original parent, unless that original parent is an, in which
	 * case we will start rebalancing from the victim node itself
	 * (after it has replaced an).
	 */
	p = victim->parent;
	if (p == an)
		p = victim;

	/*
	 * Point an's parent's pointer to it to victim, move an's
	 * children to victim, and make an's children point back to
	 * victim as their parent.
	 */
	replace_reference(tree, an, victim);
	victim->left = an->left;
	victim->right = an->right;
	victim->parent = an->parent;
	victim->height = an->height;
	if (victim->left != NULL)
		victim->left->parent = victim;
	if (victim->right != NULL)
		victim->right->parent = victim;

	return p;
}

avl_node_t * avl_tree_find_node(avl_tree_t *tree, void *data)
{
	if (!tree->root)
		return NULL;

	avl_node_t * res = tree->root;

	while (res != NULL)
	{
		int cmp = tree->compare(res->data, data);
		if (cmp < 0)
			res = res->right;
		else if (cmp > 0)
			res = res->left;
		else
			break;
	}

	return res;
}

void * avl_tree_find(avl_tree_t *tree, void *data)
{
	avl_node_t * f = avl_tree_find_node(tree, data);

	return f ? f->data : NULL;
}

void * avl_tree_delete(avl_tree_t *tree, void *data)
{
	if (!tree)
		return NULL;

	avl_node_t * an = avl_tree_find_node(tree, data);
	if (!an)
		return NULL;

	void * ret = an->data;

	avl_node_t *p;

	if (tree->print)
		tree->print(an->data, "delete");

	if (an->left == NULL && an->right == NULL)
		p = avl_tree_delete_leaf(tree, an);
	else
		p = avl_tree_delete_nonleaf(tree, an);

	rebalance_path(tree, p);

	p_free(tree->allocator, an);

	tree->elements--;

	return ret;
}

int avl_tree_clear_all(avl_tree_t *tree, avl_tree_free_fn_t free_function, void *cookie)
{
	int cnt = tree->elements;

	avl_node_t ** nodes_to_free = malloc(cnt * sizeof(avl_node_t *));
	if (!nodes_to_free)
		return -1;

	int i = 0;
	avl_node_t *it;
	for (it = avl_tree_first(tree); it != NULL; it = avl_tree_next(it))
	{
		nodes_to_free[i++] = it;
	}

	for (i = 0; i < cnt; i++)
	{
		avl_node_t *it = nodes_to_free[i];

		if (tree->print)
			tree->print(it->data, "delete");

		if (free_function)
			free_function(it->data, cookie);

		p_free(tree->allocator, it);
	}

	tree->elements = 0;
	tree->root = NULL;
	free(nodes_to_free);
}

avl_node_t *avl_tree_next(avl_node_t *an)
{
	avl_node_t *p;

	if (an->right != NULL)
	{
		an = an->right;

		while (an->left != NULL)
			an = an->left;

		return an;
	}

	p = an->parent;
	while (p != NULL && an == p->right)
	{
		an = p;
		p = an->parent;
	}

	return p;
}

avl_node_t *avl_tree_prev(avl_node_t *an)
{
	avl_node_t *p;

	if (an->left != NULL)
	{
		an = an->left;

		while (an->right != NULL)
			an = an->right;

		return an;
	}

	p = an->parent;
	while (p != NULL && an == p->left)
	{
		an = p;
		p = an->parent;
	}

	return p;
}

avl_node_t * avl_tree_first(const avl_tree_t *tree)
{
	if (!tree)
		return NULL;

	if (!tree->root)
		return NULL;

	avl_node_t *an = tree->root;

	while (an->left != NULL)
		an = an->left;

	return an;
}

avl_node_t * avl_tree_last(const avl_tree_t *tree)
{
	if (!tree)
		return NULL;

	if (!tree->root)
		return NULL;

	avl_node_t *an = tree->root;

	while (an->right != NULL)
		an = an->right;

	return an;
}

avl_tree_t * create_avl_tree(avl_tree_compare_fn_t compare, avl_tree_print_fn_t print)
{
	avl_tree_t * ret = calloc(1, sizeof(avl_tree_t));

	ret->compare = compare;
	ret->print = print;
	ret->allocator = create_pool_allocator(sizeof(avl_node_t), AVL_MEM_CHUNK_SIZE);

	return ret;
}

void delete_avl_tree(avl_tree_t * tree)
{
	if (!tree)
		return;

	if (tree->allocator)
		delete_pool_allocator(tree->allocator);

	free(tree);
}
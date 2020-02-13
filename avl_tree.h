#ifndef __AVL_H
#define __AVL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

#include <pool_allocator.h>

#define AVL_MEM_CHUNK_SIZE 1000

struct avl_node
{
	struct avl_node	*left;
	struct avl_node	*right;
	struct avl_node	*parent;
	void            *data;

	uint8_t		    height;
};

typedef int (*avl_tree_compare_fn_t)(void *a, void *b);
typedef int	(*avl_tree_print_fn_t)(void *node, char * operation);
typedef void (*avl_tree_free_fn_t)(void *a, void *cookie);

struct avl_tree
{
	avl_tree_compare_fn_t compare;
	avl_tree_print_fn_t print;

	memory_pool_allocator_t * allocator;

	uint32_t elements;
	struct avl_node	*root;
};

typedef struct avl_node avl_node_t;
typedef struct avl_tree avl_tree_t;

avl_tree_t * create_avl_tree(avl_tree_compare_fn_t compare, avl_tree_print_fn_t print);
void delete_avl_tree(avl_tree_t * tree);

int avl_tree_insert(avl_tree_t *tree, void *data);
void * avl_tree_delete(avl_tree_t *tree, void *data);

void *avl_tree_find(avl_tree_t *tree, void *data);
int avl_tree_clear_all(avl_tree_t *tree, avl_tree_free_fn_t free_function, void *cookie);

avl_node_t *avl_tree_first(const avl_tree_t *tree);
avl_node_t *avl_tree_last(const avl_tree_t *tree);
avl_node_t *avl_tree_next(avl_node_t *an);
avl_node_t *avl_tree_prev(avl_node_t *an);

/*
#define avl_tree_for_each(an, tree) \
	for (an = avl_tree_first(tree); an != NULL; an = avl_tree_last(an))

static inline avl_node_t *avl_tree_next_safe(avl_node_t *an)
{
	return an != NULL ? avl_tree_next(an) : NULL;
}

#define avl_tree_for_each_safe(an, an2, tree) \
	for (an = avl_tree_first(tree), an2 = avl_tree_next_safe(an); \
	     an != NULL; an = an2, an2 = avl_tree_next_safe(an))
*/

#ifdef __cplusplus
}
#endif


#endif

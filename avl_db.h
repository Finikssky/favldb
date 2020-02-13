#ifndef _AVL_DB_H
#define _AVL_DB_H

#include <avl_tree.h>

typedef struct
{
	
} avl_db_t;

avl_db_t * avl_db_create();
void avl_db_delete(avl_db_t *db);
void avl_db_add_index(uint32_t id, avl_tree_compare_fn_t compare, avl_tree_print_fn_t print);

int avl_db_insert_node(avl_db_t *db, void *data);
void * avl_db_delete_node(avl_db_t *db, void *data);

#endif
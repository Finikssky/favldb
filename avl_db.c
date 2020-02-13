#include <avl_db.h>

avl_db_t * avl_db_create()
{
	avl_db_t * ret = calloc(1, sizeof(avl_db_t));
	if (!ret)
		return NULL;

	return ret;
}

void avl_db_delete(avl_db_t *db)
{
	free(db);
}

void avl_db_add_index(uint32_t id, avl_tree_compare_fn_t compare, avl_tree_print_fn_t print)
{

}

int avl_db_insert_node(avl_db_t *db, void *data)
{

}

void * avl_db_delete_node(avl_db_t *db, void *data)
{

}
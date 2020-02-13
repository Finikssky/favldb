#include <pool_allocator.h>

#include <stdlib.h>
#include <string.h>

void mpl_add_pool(memory_pool_list_t *list, memory_pool_t *pool)
{
	if (list->head == NULL)
	{
		list->head = pool;
		list->tail = pool;
	}
	else
	{
		list->tail->next = pool;
		pool->prev = list->tail;
	}

	list->count++;
}

void mpl_remove_pool(memory_pool_list_t *list, memory_pool_t *pool)
{
	if (list->head == pool)
	{
		list->head = pool->next;
	}

	if (list->tail == pool)
	{
		list->tail = pool->prev;
	}

	if (pool->prev)
		pool->prev->next = pool->next;

	if (pool->next)
		pool->next->prev = pool->prev;

	pool->next = NULL;
	pool->prev = NULL;

	list->count--;
}

memory_pool_allocator_t *create_pool_allocator(uint32_t element_size, uint32_t pool_size)
{
	memory_pool_allocator_t *ret = calloc(1, sizeof(memory_pool_allocator_t));
	if (!ret)
		return NULL;

	ret->element_size = element_size + sizeof(memory_pool_element_t);
	ret->pool_size = pool_size;

	return ret;
}

void delete_pool_allocator(memory_pool_allocator_t *allocator)
{
	if (!allocator)
		return;

	memory_pool_t *pool = NULL;

	for (pool = allocator->available_pools.head; pool != NULL; pool = allocator->available_pools.head)
	{
		mpl_remove_pool(&allocator->available_pools, pool);
		if (pool->data)
			free(pool->data);

		free(pool);
	}

	for (pool = allocator->full_pools.head; pool != NULL; pool = allocator->full_pools.head)
	{
		mpl_remove_pool(&allocator->full_pools, pool);
		if (pool->data)
			free(pool->data);

		free(pool);
	}

	free(allocator);
}

int init_pool(memory_pool_t *pool, uint32_t element_size, uint32_t pool_size)
{
	pool->data = calloc(pool_size, element_size);
	if (!pool->data)
		return -1;

	pool->available = pool_size;
	pool->head = pool->data;

	for (int i = 0; i < (pool_size - 1); i++)
	{
		memory_pool_element_t *current = (memory_pool_element_t *)(pool->data + i * element_size);

		current->next = (void *)current + element_size;
	}

	return 0;
}

memory_pool_t *add_pool(memory_pool_allocator_t *allocator)
{
	memory_pool_t *pool = calloc(1, sizeof(memory_pool_t));
	if (!pool)
	{
		return NULL;
	}

	if (0 != init_pool(pool, allocator->element_size, allocator->pool_size))
	{
		free(pool);
		return NULL;
	}

	mpl_add_pool(&allocator->available_pools, pool);

	return pool;
}

void *p_alloc(memory_pool_allocator_t *allocator)
{
	memory_pool_t *pool = allocator->available_pools.head;
	if (!pool)
	{
		pool = add_pool(allocator);
		if (!pool)
		{
			return NULL;
		}
	}

	memory_pool_element_t *element = pool->head;
	pool->head = element->next;
	element->pool = pool;

	pool->available--;

	if (pool->available == 0)
	{
		mpl_remove_pool(&allocator->available_pools, pool);
		mpl_add_pool(&allocator->full_pools, pool);
	}

	return (void *)element + sizeof(memory_pool_element_t);
}

void p_free(memory_pool_allocator_t *allocator, void *data)
{
	memory_pool_element_t *element = (memory_pool_element_t *)(data - sizeof(memory_pool_element_t));
	memory_pool_t *pool = (memory_pool_t *)element->pool;

	memset(element, 0, allocator->element_size); //?

	element->next = pool->head;
	pool->head = element;

	pool->available++;

	if (pool->available == 1)
	{
		mpl_remove_pool(&allocator->full_pools, pool);
		mpl_add_pool(&allocator->available_pools, pool);
	}
}
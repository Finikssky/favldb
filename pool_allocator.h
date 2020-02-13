#ifndef _H_POOL_ALLOC
#define _H_POOL_ALLOC

#include <inttypes.h>

typedef struct
{
	union
	{
		void *next;
		void *pool;
	};
} memory_pool_element_t;

typedef struct memory_pool_s
{
	uint32_t available;

	memory_pool_element_t *head;

	void *data;

	struct memory_pool_s *next;
	struct memory_pool_s *prev;
} memory_pool_t;

typedef struct memory_pool_list_s
{
	memory_pool_t *head;
	memory_pool_t *tail;
	uint32_t count;
} memory_pool_list_t;

typedef struct
{
	uint32_t element_size;
	uint32_t pool_size;

	memory_pool_list_t available_pools;
	memory_pool_list_t full_pools;
} memory_pool_allocator_t;

memory_pool_allocator_t *create_pool_allocator(uint32_t element_size, uint32_t pool_size);
void delete_pool_allocator(memory_pool_allocator_t *allocator);

void *p_alloc(memory_pool_allocator_t *allocator);
void p_free(memory_pool_allocator_t *allocator, void *data);

#endif
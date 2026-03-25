#include <sus/hashtable.h>
#include <sus/hashset.h>
#include <sus/hashes.h>

#include <stdlib.h>

//hashtable_t<void*, size_t>
static hashtable_t *allocs;

void gcg_init()
{
	allocs = hashtable_create(hash_ptr, compare_ptr);
}

void *gcg_malloc(size_t size)
{
	void *ptr = malloc(size);

	hashtable_add(allocs, ptr, (void*)size);

	return ptr;
}

void *gcg_free(void *ptr)
{
	//TODO: Implement tracking
	free(ptr);
}

static void *get_sp();
static void *get_stack_base();

#include <stdio.h>

/// @brief Populates marked with the allocs found to be referenced
/// @param marked hashset_t<void*>
static void gcg_mark(hashset_t *marked)
{
	void *stack_pointer = get_sp();
	void *stack_base = get_stack_base();
	printf("[GCG] Marking from stack [%p, %p]\n", stack_pointer, stack_base);

	uint8_t *base;
	size_t size;

	//ivector_t<void*>
	ivector_t *ptrs = hashtable_list_keys(allocs);
	//ivector_t<size_t>
	ivector_t *sizes = hashtable_list_contents(allocs);

	size_t alloc_count = ivector_get_count(ptrs);

	for (void **cur_stack = stack_pointer; cur_stack < stack_base; ++cur_stack)
	{
		void *ptr = *cur_stack;
		printf("[CGC] Checking %p at %p\n", ptr, cur_stack);

		for (size_t i = 0; i < alloc_count; ++i)
		{
			ivector_fetch(ptrs, i, &base);
			ivector_fetch(ptrs, i, &size);

			if (ptr < base) continue;
			if (ptr > base+size) continue;

			if (hashset_contains(marked, base))
				continue;

			printf("[CGC] Marked %p due to %p\n", base, ptr);
			hashset_add(marked, base);
			//TODO: Search inside alloc for more pointers
		}
	}

	ivector_destroy(ptrs);
	ivector_destroy(sizes);
}

/// @param marked hashset_t<void*>
static void gcg_sweep(hashset_t *marked)
{

}

void gcg_collect()
{
	//hashset_t<void*>
	hashset_t *marked = hashset_create(hash_ptr, compare_ptr);

	gcg_mark(marked);
	gcg_sweep(marked);

	hashset_destroy(marked);
}

static void *get_sp()
{
	return __builtin_frame_address(0);
}

#define _GNU_SOURCE
#include <pthread.h>

static void *get_stack_base()
{
	void *base;
	size_t size;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_getstack(&attr, &base, &size);
    pthread_attr_destroy(&attr);
	return base;
}

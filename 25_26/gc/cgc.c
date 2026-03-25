#include <sus/hashtable.h>
#include <sus/hashset.h>
#include <sus/hashes.h>

#include <stdlib.h>
#include <stdio.h>

//hashtable_t<void*, size_t>
static hashtable_t *allocs = NULL;

static void cgc_cleanup();
static void cgc_init()
{
	allocs = hashtable_create(hash_ptr, compare_ptr);
	atexit(cgc_cleanup);
}

static void cgc_cleanup()
{
	//Run normal free to cleanup left-over allocs
	hashtable_destroy_free(allocs, free, NULL);
}

void *cgc_malloc(size_t size)
{
	if (allocs == NULL)
		cgc_init();

	void *ptr = malloc(size);

	hashtable_add(allocs, ptr, (void*)size);

	return ptr;
}

static void _inner_free(void *ptr)
{
	hashtable_remove(allocs, ptr, NULL, NULL);
	free(ptr);
}

void *cgc_free(void *ptr)
{
	if (allocs == NULL)
		cgc_init();

	_inner_free(ptr);
}

#define get_sp() __builtin_frame_address(0)
static void *get_stack_base();

/// @brief Populates marked with the allocs found to be referenced
/// @param marked hashset_t<void*>
static void cgc_mark(hashset_t *marked, void *stack_pointer)
{
	void *stack_base = get_stack_base();

	uint8_t *alloc_base;
	size_t alloc_size;

	//ivector_t<void*>
	ivector_t *ptrs = hashtable_list_keys(allocs);
	//ivector_t<size_t>
	ivector_t *sizes = hashtable_list_contents(allocs);

	size_t alloc_count = ivector_get_count(ptrs);

	for (void **cur_stack = stack_pointer; (void*)cur_stack < stack_base; ++cur_stack)
	{
		void *ptr = *cur_stack;
		if (ptr == NULL || ptr < (void*)0x10000) continue;

		for (size_t i = 0; i < alloc_count; ++i)
		{
			ivector_fetch(ptrs, i, &alloc_base);
			ivector_fetch(sizes, i, &alloc_size);

			if (ptr < (void*)alloc_base) continue;
			if (ptr > (void*)(alloc_base+alloc_size)) continue;

			if (hashset_contains(marked, alloc_base))
				continue;

			hashset_add(marked, alloc_base);
			//TODO: Search inside alloc for more pointers
		}
	}

	ivector_destroy(ptrs);
	ivector_destroy(sizes);
}

/// @param marked hashset_t<void*>
static void cgc_sweep(hashset_t *marked)
{
	//ivector_t<void *>
	ivector_t *all_allocs = hashtable_list_keys(allocs);

	size_t alloc_count = ivector_get_count(all_allocs);
	size_t marked_count = hashset_get_count(marked);
	size_t to_sweep = alloc_count-marked_count;
	printf("[CGC] Sweeping %lu of %lu allocs\n", to_sweep, alloc_count);

	void **allocs_v = ivector_as_pointer(all_allocs);
	for (size_t i = 0; i < alloc_count && to_sweep != 0; ++i)
	{
		if (!hashset_contains(marked, allocs_v[i]))
			_inner_free(allocs_v[i]);
	}

	ivector_destroy(all_allocs);
}

void cgc_collect()
{
	//hashset_t<void*>
	hashset_t *marked = hashset_create(hash_ptr, compare_ptr);

	void *stack_pointer = get_sp();
	cgc_mark(marked, stack_pointer);
	cgc_sweep(marked);

	hashset_destroy(marked);
}

#define __USE_GNU
#include <pthread.h>

static void *get_stack_base()
{
	void *base;
	size_t size;
    pthread_attr_t attr;
	pthread_getattr_np(pthread_self(), &attr);
    pthread_attr_getstack(&attr, &base, &size);
    pthread_attr_destroy(&attr);
	return (uint8_t*)base+size;
}

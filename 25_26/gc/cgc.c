#include <sus/hashtable.h>
#include <sus/hashset.h>
#include <sus/hashes.h>

#include <stdlib.h>
#include <stdio.h>

//hashtable_t<void*, size_t>
static hashtable_t *allocs;

void cgc_init()
{
	allocs = hashtable_create(hash_ptr, compare_ptr);
}

void *cgc_malloc(size_t size)
{
	void *ptr = malloc(size);
	printf("[CGC] %p=malloc(%lu)\n", ptr, size);

	hashtable_add(allocs, ptr, (void*)size);

	return ptr;
}

void *cgc_free(void *ptr)
{
	//TODO: Implement tracking
	free(ptr);
}

#define get_sp() __builtin_frame_address(0)
static void *get_stack_base();

/// @brief Populates marked with the allocs found to be referenced
/// @param marked hashset_t<void*>
static void cgc_mark(hashset_t *marked, void *stack_pointer)
{
	void *stack_base = get_stack_base();
	printf("[GCG] Marking from stack [%p, %p] (0x%016X)\n", stack_pointer, stack_base, stack_pointer-stack_base);

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
		//printf("[CGC] Checking %p at %p\n", ptr, cur_stack);

		for (size_t i = 0; i < alloc_count; ++i)
		{
			ivector_fetch(ptrs, i, &alloc_base);
			ivector_fetch(sizes, i, &alloc_size);

			if (ptr < (void*)alloc_base) continue;
			if (ptr > (void*)(alloc_base+alloc_size)) continue;

			if (hashset_contains(marked, alloc_base))
			{
				//printf("[CGC] Repeated find on %p due to %p\n", alloc_base, ptr);
				continue;
			}

			printf("[CGC] Marked %p (0x%0X) due to %p at %p\n", alloc_base, alloc_size, ptr, cur_stack);
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
	size_t alloc_count = hashtable_get_count(allocs);
	size_t marked_count = hashset_get_count(marked);
	size_t to_sweep = alloc_count-marked_count;
	printf("[CGC] Sweeping %lu of %lu allocs\n", to_sweep, alloc_count);
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

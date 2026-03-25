#include <sus/hashtable.h>
#include <sus/hashset.h>
#include <sus/hashes.h>

#include <stdlib.h>
#include <stdio.h>



// === Internal state ===

//hashtable_t<void*, size_t>
static hashtable_t *allocs = NULL;
static void *stack_base = NULL;



// === Private declarations ===

static void _init();
static void _cleanup();
static void _inner_free(void *ptr);
static void *get_stack_base();

static void _collect();
static void _mark(hashset_t *marked, void *stack_pointer);
static void _sweep(hashset_t *marked);
static void _recursive_mark(hashset_t *marked, void *ptr, void **ptr_v, size_t *size_v, size_t alloc_count, int lvl);



// === Public implementations ===

void *cgc_malloc(size_t size)
{
	if (allocs == NULL)
		_init();

	// Do not tolerate 0 sized allocs
	// (void*)0 == NULL and this is used for other purposes
	if (size == 0)
		return NULL;

	//REVIEW: It's probably better to clear memory to avoid having left over pointers count towards marks
	void *ptr = malloc(size);

	hashtable_add(allocs, ptr, (void*)size);

	return ptr;
}

void cgc_collect()
{
	_collect();
}

void *cgc_free(void *ptr)
{
	if (allocs == NULL)
		_init();

	_inner_free(ptr);
}



// === Private implementations ===

static void _init()
{
	allocs = hashtable_create(hash_ptr, compare_ptr);
	stack_base = get_stack_base();
	atexit(_cleanup);
}

static void _cleanup()
{
	//Run normal free to cleanup left-over allocs
	hashtable_destroy_free(allocs, free, NULL);
}

static void _inner_free(void *ptr)
{
	hashtable_remove(allocs, ptr, NULL, NULL);
	free(ptr);
}

#define get_sp() __builtin_frame_address(0)

static void _collect()
{
	//hashset_t<void*>
	hashset_t *marked = hashset_create(hash_ptr, compare_ptr);

	void *stack_pointer = get_sp();
	_mark(marked, stack_pointer);
	_sweep(marked);

	hashset_destroy(marked);
}

/// @brief Populates marked with the allocs found to be referenced
/// @param marked hashset_t<void*>
static void _mark(hashset_t *marked, void *stack_pointer)
{
	printf("[CGC] Stack marking from %p to %p\n", stack_pointer, stack_base);

	//ivector_t<void*>
	ivector_t *ptrs = hashtable_list_keys(allocs);
	void **ptr_v = ivector_as_pointer(ptrs);
	//ivector_t<size_t>
	ivector_t *sizes = hashtable_list_contents(allocs);
	size_t *size_v = ivector_as_pointer(sizes);

	size_t alloc_count = ivector_get_count(ptrs);

	for (void **cur_stack = stack_pointer; cur_stack < (void**)stack_base; ++cur_stack)
	{
		void *ptr = *cur_stack;

		// Skip stack pointers (still a lot of them remain, how to fix?)
		if (ptr < stack_base && ptr > stack_pointer)
			continue;

		_recursive_mark(marked, ptr, ptr_v, size_v, alloc_count, 0);
	}

	ivector_destroy(ptrs);
	ivector_destroy(sizes);
}

/// @param marked hashset_t<void*>
static void _sweep(hashset_t *marked)
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
		{
			_inner_free(allocs_v[i]);
			--to_sweep;
		}
	}

	ivector_destroy(all_allocs);
}

static void _recursive_mark(hashset_t *marked, void *ptr, void **ptr_v, size_t *size_v, size_t alloc_count, int lvl)
{
	uint8_t *alloc_base;
	size_t alloc_size;
	size_t i = 0;

	// Skip NULL and low values (likely integer)
	if (ptr < (void*)0x10000)
		return;

	// Skip base of allocations already marked without search
	if (hashset_contains(marked, ptr))
		return;

	// Mark base of allocations without linear search
	if ((alloc_size = (size_t)hashtable_get(allocs, ptr)) != 0) // Relies on no zero-sized allocs
	{
		hashset_add(marked, ptr);
	}
	// Linear search in case of not being a base
	else for (i = 0; i < alloc_count; ++i)
	{
		alloc_base = ptr_v[i];
		alloc_size = size_v[i];

		if (ptr < (void*)alloc_base) continue;
		if (ptr > (void*)(alloc_base+alloc_size)) continue;

		//Already marked
		if (hashset_contains(marked, alloc_base))
			return;

		hashset_add(marked, alloc_base);
		break;
	}

	if (i == alloc_count) //No match found in linear search
		return;

	for (void **nested_addr = (void**)alloc_base; nested_addr < (void**)(alloc_base+alloc_size); ++nested_addr)
	{
		void *nested_ptr = *nested_addr;
		_recursive_mark(marked, nested_ptr, ptr_v, size_v, alloc_count, lvl+1);
	}
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

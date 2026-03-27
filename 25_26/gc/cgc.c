#include <sus/hashtable.h>
#include <sus/hashset.h>
#include <sus/hashes.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>



// === Internal state ===

//hashtable_t<void*, size_t>
static hashtable_t *allocs = NULL;
static void *stack_base = NULL;

// Metrics for auto collection
static size_t allocs_since_collect = 0;
static size_t total_allocated = 0;
static size_t allocd_size_since_collect = 0;



// === Private declarations ===

#define get_bp() __builtin_frame_address(0)

static void _init();
static void _cleanup();
static void *_inner_alloc(size_t size);
static void _inner_free(void *ptr);
static void *get_stack_base();

static void _check_collect(void *stack_pointer);
static void _collect(void *stack_pointer);
static void _mark(hashset_t *marked, void *stack_pointer);
static void _sweep(hashset_t *marked);
static void _recursive_mark(hashset_t *marked, void *ptr, void **ptr_v, size_t *size_v, size_t alloc_count, int lvl);

enum TERM_COLOR
{
	COLOR_DEFAULT = 39,
	COLOR_BLACK = 30,
	COLOR_DARK_RED= 31,
	COLOR_DARK_GREEN = 32,
	COLOR_DARK_YELLOW = 33,
	COLOR_DARK_BLUE = 34,
	COLOR_DARK_MAGENTA = 35,
	COLOR_DARK_CYAN = 36,
	COLOR_LIGHT_GRAY = 37,
	COLOR_DARK_GRAY = 90,
	COLOR_RED = 91,
	COLOR_GREEN = 92,
	COLOR_ORANGE = 93,
	COLOR_BLUE = 94,
	COLOR_MAGENTA = 95,
	COLOR_CYAN = 96,
	COLOR_WHITE = 97,
};

static void set_color(int fg, int bg, char bold);



// === Public implementations ===

void *cgc_malloc(size_t size)
{
	//TODO: Reorder check_collect to collect before large alloc

	if (allocs == NULL)
		_init();

	// Do not tolerate 0 sized allocs
	// (void*)0 == NULL and this is used for other purposes
	if (size == 0)
		return NULL;

	// Collect memory as needed
	void *stack_pointer = get_bp();
	_check_collect(stack_pointer);

	return _inner_alloc(size);
}

void cgc_collect()
{
	void *stack_pointer = get_bp();
	_collect(stack_pointer);
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

static void *_inner_alloc(size_t size)
{
	// Allocate the memory
	void *ptr = malloc(size);

	//It's probably better to clear memory to avoid having left over pointers count towards marks
	memset(ptr, 0, size);

	set_color(COLOR_DARK_BLUE, COLOR_DEFAULT, 0);
	printf("[CGC] %p=malloc(%lu)\n", ptr, size);
	set_color(COLOR_DEFAULT, COLOR_DEFAULT, 0);

	// Track allocation
	hashtable_add(allocs, ptr, (void*)size);

	// Update metrics for auto-collect
	total_allocated += size;
	allocd_size_since_collect += size;
	++allocs_since_collect;

	return ptr;
}

static void _inner_free(void *ptr)
{
	// Free the memory
	free(ptr);

	// Untrack allocation
	size_t size;
	hashtable_remove(allocs, ptr, NULL, (void**)&size);

	// Update metrics for auto-collect
	total_allocated -= size;
}

static void _check_collect(void *stack_pointer)
{
	// Collect if:
	//   - Doubled allocated memory
	//   - Doubled tracked allocation count

	if ((allocd_size_since_collect * 2) <= total_allocated && // Not doubled memory
		(allocs_since_collect * 2) <= hashtable_get_count(allocs)) // Not doubled allocs
		return;

	printf("[CGC] Autocollect (size=%lu/%lu; count=%lu/%lu)\n", allocd_size_since_collect, total_allocated, allocs_since_collect, hashtable_get_count(allocs));
	_collect(stack_pointer);
}

static void _collect(void *stack_pointer)
{
	//hashset_t<void*>
	hashset_t *marked = hashset_create(hash_ptr, compare_ptr);

	_mark(marked, stack_pointer);
	_sweep(marked);

	// Cleanup
	hashset_destroy(marked);

	// Reset metrics for auto-collect
	allocs_since_collect = 0;
	allocd_size_since_collect = 0;
}

/// @brief Populates marked with the allocs found to be referenced
/// @param marked hashset_t<void*>
static void _mark(hashset_t *marked, void *stack_pointer)
{
	//printf("[CGC] Stack marking from %p to %p\n", stack_pointer, stack_base);

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

		// Skip stack pointers
		//REVIEW: still a lot of them remain, how to fix?
		if (ptr < stack_base && ptr > stack_pointer)
			continue;

		if (ptr != NULL && ptr > (void*)0x500000000000 && ptr < (void*)0x7f0000000000)
		{
			set_color(COLOR_DARK_GRAY, COLOR_DEFAULT, 0);
			printf("[CGC] Checking stack pointer at %p: %p\n", cur_stack, ptr);
			set_color(COLOR_DEFAULT, COLOR_DEFAULT, 0);
		}
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
	set_color(COLOR_GREEN, COLOR_DEFAULT, 0);
	printf("[CGC] Sweeping %lu of %lu allocs\n", to_sweep, alloc_count);
	set_color(COLOR_DEFAULT, COLOR_DEFAULT, 0);

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
		alloc_base = ptr;
		hashset_add(marked, ptr);
		printf("[CGC] Marked base %p (lvl=%d)\n", alloc_base, lvl);
	}
	// Linear search in case of not being a base
	else for (i = 0; i < alloc_count; ++i)
	{
		alloc_base = ptr_v[i];
		alloc_size = size_v[i];

		if (ptr < (void*)alloc_base) continue;
		if (ptr >= (void*)(alloc_base+alloc_size)) continue;

		//Already marked
		if (hashset_contains(marked, alloc_base))
			return;

		hashset_add(marked, alloc_base);
		printf("[CGC] Marked %p by offset %p (lvl=%d)\n", alloc_base, ptr, lvl);
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

static void set_color(int fg, int bg, char bold)
{
	printf("\033[%d;%dm\033[%dm", bold, fg, bg + 10);
}

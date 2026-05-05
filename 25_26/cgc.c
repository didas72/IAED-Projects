#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _GNU_SOURCE
#include <dlfcn.h>

#include <sus/hashtable.h>
#include <sus/hashset.h>
#include <sus/hashes.h>


// === Internal macros ===
#define NO_GC 0
#define DO_GC 1
#define CGC_PUBLIC_ENTER(do_gc) do { \
	in_cgc = 1; \
	if (allocs == NULL) { \
		init(); \
	} \
	if (do_gc) { \
		check_collect(get_bp()); \
	} \
} while(0)
#define CGC_PUBLIC_EXIT() do { in_cgc = 0; } while(0)

#ifdef USE_DEBUG_LOG
#define LOG(fmt...) do { printf(fmt); } while(0)
#define LOG_COLOR(fg, bg, bold, fmt...) do { set_color(fg, bg, bold); LOG(fmt); set_color(COLOR_DEFAULT, COLOR_DEFAULT, 0); } while (0)
#else
// Empty on purpose
#define LOG(fmt...)
#define LOG_COLOR(fg, bg, bold, fmt...)
#endif



// === Internal state ===

// Actual libc memory functions
static void *(*real_malloc)(size_t) = NULL;
static void *(*real_calloc)(size_t, size_t) = NULL;
static void (*real_free)(void *) = NULL;
static void *(*real_realloc)(void *, size_t) = NULL;
static void *(*real_reallocarray)(void *, size_t, size_t) = NULL;

// Recursion guards (mostly for libsus)
static int in_cgc = 0;

//hashtable_t<void*, size_t>
static hashtable_t *allocs = NULL;
static void *stack_base = NULL;

// Metrics for auto collection
static size_t allocs_since_collect = 0;
static size_t total_allocated = 0;
static size_t allocd_size_since_collect = 0;



// === Private declarations ===

#define get_bp() __builtin_frame_address(0)

static void init();
static void cleanup();
static void *inner_realloc(void *ptr, size_t size);
static void inner_free(void *ptr);
static void *get_stack_base();

static void check_collect(void *stack_pointer);
static void collect(void *stack_pointer);
static void mark(hashset_t *marked, void *stack_pointer);
static void sweep(hashset_t *marked);
static void recursive_mark(hashset_t *marked, void *ptr, void **ptr_v, size_t *size_v, size_t alloc_count, int lvl);

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

void *malloc(size_t size)
{
	if (in_cgc)
	{
		return real_malloc(size);
	}

	CGC_PUBLIC_ENTER(DO_GC);
	void *new_ptr;

	if (size == 0)
	{
		new_ptr = NULL;
		goto _malloc_skip;
	}

	new_ptr = inner_realloc(NULL, size);

	//It's probably better to clear memory to avoid having left-over pointers count towards marks
	memset(new_ptr, 0, size);

_malloc_skip:
	CGC_PUBLIC_EXIT();
	return new_ptr;
}

void free(void *ptr)
{
	if (in_cgc)
	{
		real_free(ptr);
		return;
	}

	CGC_PUBLIC_ENTER(NO_GC);

	inner_free(ptr);

	CGC_PUBLIC_EXIT();
}

void *calloc(size_t n, size_t size)
{
	if (in_cgc)
	{
		return real_calloc(n, size);
	}

	CGC_PUBLIC_ENTER(DO_GC);
	void *new_ptr;

	// Prevent overflow
	if (n > SIZE_MAX / size)
	{
		new_ptr = NULL;
		goto _calloc_skip;
	}

	size_t final_size = n * size;
	if (final_size == 0)
	{
		new_ptr = NULL;
		goto _calloc_skip;
	}

	new_ptr = inner_realloc(NULL, size);
	memset(new_ptr, 0, size);

_calloc_skip:
	CGC_PUBLIC_EXIT();
	return new_ptr;
}

void *realloc(void *p, size_t size)
{
	if (in_cgc)
	{
		return real_realloc(p, size);
	}

	CGC_PUBLIC_ENTER(DO_GC);

	void *new_ptr;
	if (size == 0)
	{
		inner_free(p);
		new_ptr = NULL;
		goto _realloc_skip;
	}

	new_ptr = inner_realloc(p, size);

_realloc_skip:
	CGC_PUBLIC_EXIT();
	return new_ptr;
}

void *reallocarray(void *p, size_t n, size_t size)
{
	if (in_cgc)
	{
		return real_reallocarray(p, n, size);
	}

	CGC_PUBLIC_ENTER(DO_GC);
	void *new_ptr;

	// Prevent overflow
	if (n > SIZE_MAX / size)
	{
		new_ptr = NULL;
		goto _reallocarray_skip;
	}

	size_t final_size = n * size;
	if (final_size == 0)
	{
		inner_free(p);
		new_ptr = NULL;
		goto _reallocarray_skip;
	}

	new_ptr = inner_realloc(p, final_size);

_reallocarray_skip:
	CGC_PUBLIC_EXIT();
	return new_ptr;
}



// === Private implementations ===

static void init()
{
	// Real function setup MUST be the first thing to run
	real_malloc = dlsym(RTLD_NEXT, "malloc");
	real_calloc = dlsym(RTLD_NEXT, "calloc");
	real_free = dlsym(RTLD_NEXT, "free");
	real_realloc = dlsym(RTLD_NEXT, "realloc");
	real_reallocarray = dlsym(RTLD_NEXT, "reallocarray");

	allocs = hashtable_create(hash_ptr, compare_ptr);
	stack_base = get_stack_base();

	atexit(cleanup);
}

static void cleanup()
{
	//NOTE: While not 'public' interface, still needs in_cgc guard
	CGC_PUBLIC_ENTER(NO_GC);

	LOG_COLOR(COLOR_DARK_MAGENTA, COLOR_DEFAULT, 0, "[CGC] Cleanup at shutdown\n");

	//Run normal free to cleanup left-over allocs
	hashtable_destroy_free(allocs, real_free, NULL);

	CGC_PUBLIC_EXIT();
}

//NOTE: Assumes size will never be zero, and therefor will NOT handle 'free' behaviour
static void *inner_realloc(void *ptr, size_t size)
{
	// Find original size (if existing)
	size_t old_size = 0;
	if (hashtable_has_key(allocs, ptr))
		old_size = (size_t)hashtable_get(allocs, ptr);

	// Allocate the memory
	void *new_ptr = real_realloc(ptr, size);

	LOG_COLOR(COLOR_DARK_BLUE, COLOR_DEFAULT, 0, "[CGC] %p=alloc(%lu)\n", new_ptr, size);

	// Failed (re)allocations shouldn't change any state
	if (new_ptr == NULL)
		return NULL;

	// Track allocation
	if (old_size == 0)
		hashtable_add(allocs, ptr, (void*)size);
	else
		hashtable_set(allocs, ptr, (void*)size);

	// Update metrics for auto-collect
	total_allocated += size - old_size;
	allocd_size_since_collect += size - old_size;
	++allocs_since_collect;

	return new_ptr;
}

static void inner_free(void *ptr)
{
	// Free the memory
	real_free(ptr);

	// Untrack allocation
	size_t size;
	hashtable_remove(allocs, ptr, NULL, (void**)&size);

	// Update metrics for auto-collect
	total_allocated -= size;
}

static void check_collect(void *stack_pointer)
{
	// Collect if:
	//   - Doubled allocated memory
	//   - Doubled tracked allocation count

	if ((allocd_size_since_collect * 2) <= total_allocated && // Not doubled memory
		(allocs_since_collect * 2) <= hashtable_get_count(allocs)) // Not doubled allocs
		return;

	LOG("[CGC] Autocollect (size=%lu/%lu; count=%lu/%lu)\n", allocd_size_since_collect, total_allocated, allocs_since_collect, hashtable_get_count(allocs));
	collect(stack_pointer);
}

static void collect(void *stack_pointer)
{
	//hashset_t<void*>
	hashset_t *marked = hashset_create(hash_ptr, compare_ptr);

	mark(marked, stack_pointer);
	sweep(marked);

	// Cleanup
	hashset_destroy(marked);

	// Reset metrics for auto-collect
	allocs_since_collect = 0;
	allocd_size_since_collect = 0;
}

/// @brief Populates marked with the allocs found to be referenced
/// @param marked hashset_t<void*>
static void mark(hashset_t *marked, void *stack_pointer)
{
	LOG("[CGC] Stack marking from %p to %p\n", stack_pointer, stack_base);

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
			LOG_COLOR(COLOR_DARK_GRAY, COLOR_DEFAULT, 0, "[CGC] Checking stack pointer at %p: %p\n", cur_stack, ptr);
		}
		recursive_mark(marked, ptr, ptr_v, size_v, alloc_count, 0);
	}

	ivector_destroy(ptrs);
	ivector_destroy(sizes);
}

/// @param marked hashset_t<void*>
static void sweep(hashset_t *marked)
{
	//ivector_t<void *>
	ivector_t *all_allocs = hashtable_list_keys(allocs);

	size_t alloc_count = ivector_get_count(all_allocs);
	size_t marked_count = hashset_get_count(marked);
	size_t to_sweep = alloc_count-marked_count;
	LOG_COLOR(COLOR_GREEN, COLOR_DEFAULT, 0, "[CGC] Sweeping %lu of %lu allocs\n", to_sweep, alloc_count);

	void **allocs_v = ivector_as_pointer(all_allocs);
	for (size_t i = 0; i < alloc_count && to_sweep != 0; ++i)
	{
		if (!hashset_contains(marked, allocs_v[i]))
		{
			inner_free(allocs_v[i]);
			--to_sweep;
		}
	}

	ivector_destroy(all_allocs);
}

static void recursive_mark(hashset_t *marked, void *ptr, void **ptr_v, size_t *size_v, size_t alloc_count, int lvl)
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
		LOG("[CGC] Marked base %p (lvl=%d)\n", alloc_base, lvl);
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
		LOG("[CGC] Marked %p by offset %p (lvl=%d)\n", alloc_base, ptr, lvl);
		break;
	}

	if (i == alloc_count) //No match found in linear search
		return;

	for (void **nested_addr = (void**)alloc_base; nested_addr < (void**)(alloc_base+alloc_size); ++nested_addr)
	{
		void *nested_ptr = *nested_addr;
		recursive_mark(marked, nested_ptr, ptr_v, size_v, alloc_count, lvl+1);
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

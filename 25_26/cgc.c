int first_var;

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <link.h>
#include <pthread.h>

#include <sus/hashtable.h>
#include <sus/hashset.h>
#include <sus/hashes.h>
#include <sus/sus.h>


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

#ifdef CGC_DEBUG_LOG
#define _DBG_OUT(fmt...) do { fprintf(stderr, fmt); } while (0)
#else
// Empty on purpose
#define _DBG_OUT(fmt...)
#endif

#define _SET_COLOR(fg, bg) do { _DBG_OUT("\033[0;%dm\033[%dm", fg, bg + 10); } while (0)

#define DBG_LOG_COLOR2(fg, bg, fmt...) do { _SET_COLOR(fg, bg); _DBG_OUT(fmt); _SET_COLOR(COLOR_DEFAULT, COLOR_DEFAULT); } while (0)
#define DBG_LOG_COLOR1(fg, fmt...) do { _SET_COLOR(fg, COLOR_DEFAULT); _DBG_OUT(fmt); _SET_COLOR(COLOR_DEFAULT, COLOR_DEFAULT); } while (0)

#ifdef CGC_DEBUG_TRACE
#define DBG_TRACE(fmt...) do { DBG_LOG_COLOR1(COLOR_DARK_GRAY, "[CGC] Trace: " fmt); } while(0)
#else
#define DBG_TRACE(fmt...) do { } while(0)
#endif
#define DBG_INFO(fmt...) do { DBG_LOG_COLOR1(COLOR_WHITE, "[CGC] Info: " fmt); } while(0)
#define DBG_INFO_GOOD(fmt...) do { DBG_LOG_COLOR1(COLOR_GREEN, "[CGC] Info: " fmt); } while(0)
#define DBG_INFO_CALL(fmt...) do { DBG_LOG_COLOR1(COLOR_BLUE, "[CGC] Call: " fmt); } while(0)
#define DBG_WARN(fmt...) do { DBG_LOG_COLOR1(COLOR_DARK_YELLOW, "[CGC] Warn: " fmt); } while(0)
#define DBG_ERR(fmt...) do { DBG_LOG_COLOR1(COLOR_RED, "[CGC] Warn: " fmt); } while(0)


// === Terminal colors ===

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
static void mark_span(hashset_t *marked, void *start, size_t len);
static int dl_iterate_callback(struct dl_phdr_info *info, size_t size, void *data);
static void recursive_mark(hashset_t *marked, void *ptr, void **ptr_v, size_t *size_v, size_t alloc_count, int lvl);
static void sweep(hashset_t *marked);



// === Public implementations ===

void *malloc(size_t size)
{
	if (in_cgc)
	{
		return real_malloc(size);
	}

	CGC_PUBLIC_ENTER(DO_GC);
	DBG_INFO_CALL("malloc(%lu)\n", size);
	void *new_ptr;

	if (size == 0)
	{
		new_ptr = NULL;
		goto _malloc_skip;
	}

	new_ptr = inner_realloc(NULL, size);
	DBG_INFO("malloc(%lu) -> %p\n", size, new_ptr);

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
	DBG_INFO_CALL("free(%p)\n", ptr);

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
	DBG_INFO_CALL("calloc(%lu, %lu)\n", n, size);
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
	DBG_INFO("calloc(%lu, %lu) -> %p\n", n, size, new_ptr);

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
	DBG_INFO_CALL("realloc(%p, %lu)\n", p, size);

	void *new_ptr;
	if (size == 0)
	{
		inner_free(p);
		new_ptr = NULL;
		goto _realloc_skip;
	}

	new_ptr = inner_realloc(p, size);
	DBG_INFO("realloc(%p, %lu) -> %p\n", p, size, new_ptr);

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
	DBG_INFO_CALL("reallocarray(%p, %lu, %lu)\n", p, n, size);
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
	DBG_INFO("reallocarray(%p, %lu, %lu) -> %p\n", p, n, size, new_ptr);

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

	DBG_INFO_GOOD("[CGC] Cleanup at shutdown\n");

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

	// Failed (re)allocations shouldn't change any state
	if (new_ptr == NULL)
		return NULL;

	// Track allocation
	if (old_size == 0)
		hashtable_remove(allocs, ptr, NULL, NULL);
	hashtable_add(allocs, new_ptr, (void*)size);

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
	if (hashtable_remove(allocs, ptr, NULL, (void**)&size) == SUS_ENTRY_NOT_FOUND)
		DBG_WARN("Freed pointer %p not allocated\n", ptr);

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

	DBG_INFO("[CGC] Autocollect (size=%lu/%lu; count=%lu/%lu)\n", allocd_size_since_collect, total_allocated, allocs_since_collect, hashtable_get_count(allocs));
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
	//FIXME: We don't mark pointers in registers (FML)

	DBG_INFO("Stack marking from %p to %p\n", stack_pointer, stack_base);
	mark_span(marked, stack_pointer, (char *)stack_base - (char *)stack_pointer);

	DBG_INFO("Marking bss'es and data's\n");
	// Mark .bss and .data for each loaded binary
	dl_iterate_phdr(dl_iterate_callback, marked);
}

static int dl_iterate_callback(struct dl_phdr_info *info, size_t size, void *data)
{ (void)size;
	//hashset_t<void*>
	hashset_t *marked = data;

	for (int i = 0; i < info->dlpi_phnum; ++i)
	{
		if (info->dlpi_phdr[i].p_type != PT_LOAD ||
			(info->dlpi_phdr[i].p_flags & PF_X) != 0 ||
			(info->dlpi_phdr[i].p_flags & PF_R) == 0 ||
			(info->dlpi_phdr[i].p_flags & PF_W) == 0)
			continue;

		void *segment_start = (void *)(info->dlpi_addr + info->dlpi_phdr[i].p_vaddr);

		DBG_INFO("Marking program header %d of '%s'.\n", i, info->dlpi_name);
		mark_span(marked, segment_start, info->dlpi_phdr[i].p_memsz);
	}

	return 0;
}

static void mark_span(hashset_t *marked, void *start, size_t len)
{
	void **end = (void**)((char*)start+len);

	//ivector_t<void*>
	ivector_t *ptrs = hashtable_list_keys(allocs);
	void **ptr_v = ivector_as_pointer(ptrs);
	//ivector_t<size_t>
	ivector_t *sizes = hashtable_list_contents(allocs);
	size_t *size_v = ivector_as_pointer(sizes);

	size_t alloc_count = ivector_get_count(ptrs);

	for (void **cur_stack = start; cur_stack < end; ++cur_stack)
	{
		void *ptr = *cur_stack;

		recursive_mark(marked, ptr, ptr_v, size_v, alloc_count, 0);
	}

	ivector_destroy(ptrs);
	ivector_destroy(sizes);
}

static void recursive_mark(hashset_t *marked, void *ptr, void **ptr_v, size_t *size_v, size_t alloc_count, int lvl)
{
	uint8_t *alloc_base;
	size_t alloc_size;
	size_t i = 0;

	// Skip NULL and low values (likely integer)
	if (ptr < (void*)0x10000)
	{
		if (ptr != NULL) DBG_TRACE("Not marking %p (too low)\n", ptr);
		return;
	}

	// Skip base of allocations already marked without search
	if (hashset_contains(marked, ptr))
	{
		DBG_TRACE("Not marking %pd (already marked)\n", ptr);
		return;
	}

	// Mark base of allocations without linear search
	if ((alloc_size = (size_t)hashtable_get(allocs, ptr)) != 0) // Relies on no zero-sized allocs
	{
		alloc_base = ptr;
		DBG_TRACE("[CGC] Marking base %p (lvl=%d)\n", alloc_base, lvl);
		hashset_add(marked, ptr);
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

		DBG_TRACE("[CGC] Marking %p by offset %p (lvl=%d)\n", alloc_base, ptr, lvl);
		hashset_add(marked, alloc_base);
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

/// @param marked hashset_t<void*>
static void sweep(hashset_t *marked)
{
	//ivector_t<void *>
	ivector_t *all_allocs = hashtable_list_keys(allocs);

	size_t alloc_count = ivector_get_count(all_allocs);
	size_t marked_count = hashset_get_count(marked);
	size_t to_sweep = alloc_count - marked_count;
	DBG_INFO_GOOD("[CGC] Sweeping %lu of %lu allocs\n", to_sweep, alloc_count);

	void **allocs_v = ivector_as_pointer(all_allocs);
	for (size_t i = 0; i < alloc_count && to_sweep != 0; ++i)
	{
		if (!hashset_contains(marked, allocs_v[i]))
		{
			DBG_TRACE("Sweeping %p\n", allocs_v[i]);
			inner_free(allocs_v[i]);
			--to_sweep;
		}
	}

	ivector_destroy(all_allocs);
}

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

#include "memsplit.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define FIRST_DEPTH 1



static void recursive_insert(memsplit_t *mem, size_t segment_base, size_t alloc_base, size_t alloc_tail, int depth);
static void *recursive_query(memsplit_t *mem, void *address, int depth);



memsplit_t *memsplit_create()
{
	memsplit_t *mem = malloc(sizeof(memsplit_t));

	memset(mem->children, 0, sizeof(mem->children));

	return mem;
}

void memsplit_destroy(memsplit_t *mem)
{
	//TODO: Recursive destroy
	free(mem);
}

void memsplit_insert(memsplit_t *mem, void *base, size_t size)
{
	size_t alloc_base = (size_t)base;
	size_t alloc_tail = alloc_base + size;
	recursive_insert(mem, 0, alloc_base, alloc_tail, FIRST_DEPTH);
}

void *memsplit_query(memsplit_t *mem, void *address)
{
	return recursive_query(mem, address, FIRST_DEPTH);
}

static void recursive_insert(memsplit_t *mem, size_t segment_base, size_t alloc_base, size_t alloc_tail, int depth)
{ //TODO: Insertion optimization described in doc/ (and respective change in query)
	if (depth == MEMSPLIT_DEPTH - 1)
	{
		for (size_t i = 0; i < MEMSPLIT_ENTRIES; ++i)
		{
			size_t entry_ptr = segment_base + i;
			if (alloc_base > entry_ptr || entry_ptr >= alloc_tail)
				continue;

			mem->children[i] = (void *)segment_base;
		}
		return;
	}

	size_t subsegment_length = 1 << ((MEMSPLIT_DEPTH - depth) * MEMSPLIT_BPL);

	for (size_t i = 0; i < MEMSPLIT_ENTRIES; ++i)
	{
		size_t subsegment_base = segment_base + i * subsegment_length;
		size_t subsegment_tail = segment_base + (i + 1) * subsegment_length;

		if (subsegment_tail < alloc_base || alloc_tail < subsegment_base)
			continue;

		if (mem->children[i] == NULL)
			mem->children[i] = memsplit_create();
		recursive_insert(mem->children[i], subsegment_base, alloc_base, alloc_tail, depth + 1);
	}
}

static void *recursive_query(memsplit_t *mem, void *address, int depth)
{
	if (depth == MEMSPLIT_DEPTH)
		return mem;

	size_t idx = ((size_t)address >> ((MEMSPLIT_DEPTH - depth) * MEMSPLIT_BPL)) & MEMSPLIT_MASK;
	if (mem->children[idx] == NULL)
		return NULL;

	return recursive_query(mem->children[idx], address, depth + 1);
}

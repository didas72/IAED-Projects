/*
 * File: memctrl_i.c
 * Author: Diogo Diniz (ist1106196)
 * Description: Implementation of functions in memctrl_i.h
*/

#include <stdio.h>
#include <stdlib.h>
#include "memctrl_i.h"

AllocList* initAllocList()
{
	AllocList* ret = malloc(sizeof(AllocList));
	ret->count = 0;
	ret->size = MEM_LIST_BLOCK_SIZE;
	ret->allocations = malloc(MEM_LIST_BLOCK_SIZE * sizeof(AllocEntry));

	return ret;
}
void destroyAllocList(AllocList* list)
{
	free(list->allocations);
	free(list);
}

void registerAlloc(AllocList* list, void* ptr, size_t size, char* name)
{
	AllocEntry* entry;

	if (list->count == list->size)
	{
		list->size += MEM_LIST_BLOCK_SIZE;
		list->allocations = realloc(list->allocations,
			list->size * sizeof(AllocEntry));
	}

	entry = &list->allocations[list->count++];
	entry->size = size;
	entry->status = ALLOC_USED;
	entry->ptr = ptr;
	entry->nameC = 1;
	entry->names[0] = name;
}
void registerFree(AllocList* list, void* ptr, char* name)
{
	int i = 0;
	AllocEntry* entry;

	if (!ptr)
		return;

	for (; i < list->count; i++)
	{
		if (list->allocations[i].ptr == ptr)
		{
			entry = &list->allocations[i];

			if (entry->nameC == MAX_NAMES)
			{
				printf("Reached MAX_NAME with %s.\n", name);
				exit(421);
			}

			if (entry->status == ALLOC_FREED)
			{
				printf("Freed same block twice (%s).\n", name);
				exit(420);
			}
			
			entry->ptr = NULL; /*TODO: Remove causes duplicate free
									of stops list struct*/
			entry->status = ALLOC_FREED;
			entry->names[entry->nameC++] = name;
			return;
		}
	}

	printf("Unregistered ptr for free().");
	exit(420);
}
void registerRealloc(AllocList* list, void* oldPtr, void* newPtr, size_t size,
	char* name)
{
	int i = 0;
	AllocEntry* entry;

	if (!oldPtr)
		return;

	for (; i < list->count; i++)
	{
		if (list->allocations[i].ptr == oldPtr)
		{
			entry = &list->allocations[i];

			if (entry->nameC == MAX_NAMES)
			{
				printf("Reached MAX_NAME with %s.", name);
				exit(421);
			}

			entry->status = ALLOC_USED;
			entry->ptr = newPtr;
			entry->size = size;
			return;
		}
	}

	printf("Unregistered ptr for realloc().");
	exit(420);
}

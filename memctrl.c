/*
 * File: memctrl.c
 * Author: Diogo Diniz (ist1106196)
 * Description: Implementation of functions in memctrl.h
*/

#include <stdio.h>
#include <stdlib.h>
#include "memctrl_i.h"


AllocList* getSetList(AllocList* inp)
{
	static AllocList* allocList = 0;

	if (inp)
		allocList = inp;
		
	return allocList;
}
char* getStatusString(char status)
{
	switch(status)
	{
		case ALLOC_NONE:
			return "NONE";

		case ALLOC_USED:
			return "USED";

		case ALLOC_FREED:
			return "FREED";

		default:
			return "NVAL";
	}
}

void initMemCtrl(char shouldDo)
{
	AllocList* allocList = getSetList(NULL);

	if (!shouldDo)
		return;

	if (allocList)
		return;

	allocList = initAllocList();

	getSetList(allocList);
}
void uninitMemCtrl(char shouldDo)
{
	AllocList* allocList = getSetList(NULL);

	if (!shouldDo)
		return;

	if (!allocList)
	{
		printf("memctrl not initialized.");
		exit(420);
	}

	destroyAllocList(allocList);
}
void listAllMem(char shouldDo)
{
	AllocList* allocList = getSetList(NULL);
	int i = 0, j;
	AllocEntry* entry;

	if (!shouldDo)
		return;

	if (!allocList)
	{
		printf("memctrl not initialized.");
		exit(420);
	}

	for (; i < allocList->count; i++)
	{
		entry = &allocList->allocations[i];

		printf("[%s] Block %d of final size %lu has history:",
			getStatusString(entry->status), i, entry->size);

		for (j = 0; j < entry->nameC; j++)
			printf("\n\t-'%s'", entry->names[j]);
		
		printf("\n");
	}
}
void listUsedMem(char shouldDo)
{
	AllocList* allocList = getSetList(NULL);
	int i = 0, j;
	AllocEntry* entry;

	if (!shouldDo)
		return;

	if (!allocList)
	{
		printf("memctrl not initialized.");
		exit(420);
	}

	for (; i < allocList->count; i++)
	{
		entry = &allocList->allocations[i];

		if (entry->status != ALLOC_USED)
			continue;

		printf("[%s] Block %d of final size %lu has history:",
			getStatusString(entry->status), i, entry->size);

		for (j = 0; j < entry->nameC; j++)
			printf("\n\t-'%s'", entry->names[j]);
		
		printf("\n");
	}
}
char anyNotFreed(char shouldDo)
{
	AllocList* allocList = getSetList(NULL);
	int i = 0;
	AllocEntry* entry;

	if (!shouldDo)
		return 0;

	if (!allocList)
	{
		printf("memctrl not initialized.");
		exit(420);
	}

	for (; i < allocList->count; i++)
	{
		entry = &allocList->allocations[i];

		if (entry->status != ALLOC_FREED)
			return 1;
	}

	return 0;
}

void* myMalloc(size_t size, char* name)
{
	AllocList* allocList = getSetList(NULL);
	void* ptr;
	
	if (!allocList)
	{
		printf("memctrl not initialized.");
		exit(420);
	}

	ptr = malloc(size);

	registerAlloc(allocList, ptr, size, name);

	return ptr;
}
void* myCalloc(size_t nitems, size_t size, char* name)
{
	AllocList* allocList = getSetList(NULL);
	void* ptr;

	if (!allocList)
	{
		printf("memctrl not initialized.");
		exit(420);
	}

	ptr = calloc(nitems, size);

	registerAlloc(allocList, ptr, size, name);

	return ptr;
}
void myFree(void* ptr, char* name)
{
	AllocList* allocList = getSetList(NULL);

	if (!allocList)
	{
		printf("memctrl not initialized.");
		exit(420);
	}

	free(ptr);

	registerFree(allocList, ptr, name);
}
void* myRealloc(void* ptr, size_t size, char* name)
{
	AllocList* allocList = getSetList(NULL);
	void* newPtr;

	if (!allocList)
	{
		printf("memctrl not initialized.");
		exit(420);
	}

	newPtr = realloc(ptr, size);

	registerRealloc(allocList, ptr, newPtr, size, name);

	return newPtr;
}

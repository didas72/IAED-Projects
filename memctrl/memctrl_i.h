/*
 * File: memctrl_i.h
 * Author: Diogo Diniz (ist1106196)
 * Description: Internals for implementation of memctrl header
*/

#define MEM_LIST_BLOCK_SIZE 64

#define ALLOC_NONE 0
#define ALLOC_USED 1
#define ALLOC_FREED 2

#ifndef MAX_NAMES
#define MAX_NAMES 3
#endif


typedef struct
{
	size_t size;
	void* ptr;
	char* names[MAX_NAMES];
	int nameC;
	char status;
} AllocEntry;

typedef struct
{
	AllocEntry* allocations;
	int count, size;
} AllocList;


AllocList* initAllocList();
void destroyAllocList(AllocList* list);
void registerAlloc(AllocList* list, void* ptr, size_t size, char* name);
void registerFree(AllocList* list, void* ptr, char* name);
void registerRealloc(AllocList* list, void* oldPtr, void* newPtr, size_t size,
	char* name);

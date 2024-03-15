/*
 * File: collections.h
 * Author: Diogo Diniz (ist1106196)
 * Description: Generic collction structures and functions header
*/

#ifndef COLLECTIONS_H
#define COLLECTIONS_H

/*===MACROS===*/
#define ITER_FUNC_DECL(retT) retT (*elIter)(void*, void**, int)
#define ITER_FUNC_ARGS void* content, void** args, int argC
#define ITER_FUNC_EXTRA_ARGS void** args, int argC
#define ITER_FUNC_CALL(content) elIter(content, args, argC)
#define ITER_FUNC_DISCARD_ARGS() (void)args; if (argC != 0)\
	{ ERROR0("Invalid argC.\n"); exit(-1); }
#define ITER_FUNC_CHECK_ARGC(argc) if (argC != argc) \
	{ ERROR0("Invalid argC.\n"); exit(-1); }

/*===STRUCTURES==*/
typedef struct DLinkedNode DLinkedNode;

typedef struct
{
	/*Count will help reduce access and removal time by 50%*/
	/*Only applies when using indexes :(*/
	int count; /*0-3*/
	DLinkedNode *start, *end; /*4-7;8-11*/
} DLinkedList;

struct DLinkedNode
{
	void* content; /*0-3*/
	DLinkedNode *next, *prev; /*4-7;8-11*/
};


/*===FUNCTION DECLARATION===*/
/*=Constructors and destructors=*/
/*createList - Allocates and initializes an empty DLinkedList.*/
DLinkedList* createList();
/*destroyList - Frees all nodes and base list struct.*/
void destroyList(DLinkedList* list);
/*destroyListElements - Frees all nodes, their corresponding contents
  by calling the elFree function and frees the base list struct*/
void destroyListElements(DLinkedList* list, void (*elFree)(void*));

/*=Addition and removal=*/
/*appendContentToList - Adds content pointer to the end of the list*/
void appendContentToList(DLinkedList* list, void* content);
/*prependContentToList - Adds content pointer to the start of the list*/
void prependContentToList(DLinkedList* list, void* content);
/*removeNodeFromList - Removes the given node from the list, patching the links
  and list tips.*/
void removeNodeFromList(DLinkedList* list, DLinkedNode* node);
/*removeContentFromList - Tries to remove list node with the given content.
  If found, frees the node and returns non-zero value.*/
char removeContentFromList(DLinkedList* list, void* content);

/*=Iterators=*/
/*iterateListContents - Runs a void function over all list elements*/
void iterateListContents(DLinkedList* list, char rev,
	ITER_FUNC_DECL(void), ITER_FUNC_EXTRA_ARGS);
/*iterateSumListContents - Runs a float function over all list elements
  and returns the sum of all returned values*/
double iterateSumListContents(DLinkedList* list,
	ITER_FUNC_DECL(float), ITER_FUNC_EXTRA_ARGS);
/*iterateAnyListContents - Runs a char function over all list elements
  and returns non-zero if any call returned non-zero*/
char iterateAnyListContents(DLinkedList* list,
	ITER_FUNC_DECL(char), ITER_FUNC_EXTRA_ARGS);
/*iterateCountListContents- Runs a char function over all list elements
  and returns the number of calls that returned non-zero*/
int iterateCountListContents(DLinkedList* list,
	ITER_FUNC_DECL(char), ITER_FUNC_EXTRA_ARGS);

/*=Search=*/
/*findFirstListContent - Runs a char function over all list elements
  and returns the first element whose function call returned non-zero.
  Returns NULL if no elements retuned non-zero.*/
DLinkedNode* findFirstListContent(DLinkedList* list,
	ITER_FUNC_DECL(char), ITER_FUNC_EXTRA_ARGS);

/*=Others=*/
/*sortListContents - Sorts all list elements using the given comparison
  function.*/
void sortListContents(DLinkedList* list,
	int (*compare)(void*, void*));

#endif

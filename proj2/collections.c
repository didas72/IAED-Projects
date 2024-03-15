/*
 * File: collections.c
 * Author: Diogo Diniz (ist1106196)
 * Description: Implementation of functions in collections.h
*/

#include <stdlib.h>
#include <stdio.h>

#include "collections.h"
#include "errors.h"

/*createList - Simple allocation with validation and assignement*/
DLinkedList* createList()
{
	DLinkedList* list = malloc(sizeof(DLinkedList));

	CHECK_ALLOC(list);

	list->count = 0;
	list->start = list->end = NULL;
	return list;
}
/*destroyList - Simple iterated frees*/
void destroyList(DLinkedList* list)
{
	DLinkedNode* curNode = list->start, *nextNode;

	while (curNode)
	{
		nextNode = curNode->next;
		free(curNode);
		curNode = nextNode;
	}

	free(list);
}
/*destroyListElements - Simple iterated frees and elFree calls*/
void destroyListElements(DLinkedList* list, void (*elFree)(void*))
{
	DLinkedNode* curNode = list->start, *nextNode;

	while (curNode)
	{
		elFree(curNode->content);

		nextNode = curNode->next;
		free(curNode);
		curNode = nextNode;
	}

	free(list);
}


/*appendContentToList - Create node, assign content and patch links*/
void appendContentToList(DLinkedList* list, void* content)
{
	DLinkedNode* node = malloc(sizeof(DLinkedNode));

	CHECK_ALLOC(node);

	if (!list->start)
		list->start = node;

	if (list->end)
		list->end->next = node;

	node->prev = list->end;
	node->content = content;
	node->next = NULL;
	list->end = node;
	list->count++;
}
/*prependContentToList - Create node, assign content and patch links*/
void prependContentToList(DLinkedList* list, void* content)
{
	DLinkedNode* node = malloc(sizeof(DLinkedNode));

	CHECK_ALLOC(node);
		
	if (!list->end)
		list->end = node;

	if (list->start)
		list->start->prev = node;

	node->next = list->start;
	node->content = content;
	node->prev = NULL;
	list->start = node;
	list->count++;
}
/*removeNodeFromList - Direct link patching and freeing of node. Also patch
  list start and end*/
void removeNodeFromList(DLinkedList* list, DLinkedNode* node)
{
	/*Match neighbours*/
	if (node->next)
		node->next->prev = node->prev;
	if (node->prev)
		node->prev->next = node->next;

	/*Patch list tips*/
	if (list->start == node)
		list->start = node->next;
	/*Not else if in case is the only element in the list*/
	if (list->end == node)
		list->end = node->prev;

	/*Update count*/
	list->count--;

	free(node);
}
/*removeContentFromList - Searches for content and calls removeNodeFromList*/
char removeContentFromList(DLinkedList* list, void* content)
{
	DLinkedNode* curNode = list->start;

	/*Find node with given content*/
	while (curNode)
	{
		if (curNode->content == content)
			break;
		curNode = curNode->next;
	}

	if (!curNode) /*Not found*/
		return 0;

	removeNodeFromList(list, curNode);
	return 1;
}


/*iterateListContents - Simple iteration with function call*/
void iterateListContents(DLinkedList* list, char rev,
	ITER_FUNC_DECL(void), ITER_FUNC_EXTRA_ARGS)
{
	DLinkedNode* curNode = rev ? list->end : list->start;

	while (curNode)
	{
		ITER_FUNC_CALL(curNode->content);
		curNode = rev ? curNode->prev : curNode->next;
	}
}
/*iterateSumListContents - Simple iteration with addition to total counter*/
double iterateSumListContents(DLinkedList* list,
	ITER_FUNC_DECL(float), ITER_FUNC_EXTRA_ARGS)
{
	double sum = 0.0f;
	DLinkedNode* curNode = list->start;

	while (curNode)
	{
		sum += ITER_FUNC_CALL(curNode->content);
		curNode = curNode->next;
	}

	return sum;
}
/*iterateAnyListContents - Simple iteration with function call and return
  on successful match*/
char iterateAnyListContents(DLinkedList* list,
	ITER_FUNC_DECL(char), ITER_FUNC_EXTRA_ARGS)
{
	DLinkedNode* curNode = list->start;

	while (curNode)
	{
		if (ITER_FUNC_CALL(curNode->content))
			return 1;
		curNode = curNode->next;
	}

	return 0;
}
/*iterateCountListContents - Simple iteration with function call and
  counter incrementing*/
int iterateCountListContents(DLinkedList* list,
	ITER_FUNC_DECL(char), ITER_FUNC_EXTRA_ARGS)
{
	int count = 0;
	DLinkedNode* curNode = list->start;

	while (curNode)
	{
		count += ITER_FUNC_CALL(curNode->content);
		curNode = curNode->next;
	}

	return count;
}

/*findFirstListContent - Simple iteration with function call and return
  on successful match*/
DLinkedNode* findFirstListContent(DLinkedList* list,
	ITER_FUNC_DECL(char), ITER_FUNC_EXTRA_ARGS)
{
	DLinkedNode* curNode = list->start;

	while (curNode)
	{
		if (ITER_FUNC_CALL(curNode->content))
			return curNode;
		curNode = curNode->next;
	}

	return NULL;
}

/*sortListContents - Bubble sort implementation. Moves pointers
  instead of nodes*/
void sortListContents(DLinkedList* list, 
	int (*compare)(void*, void*))
{
	/*Bubble sort, screw it. Change if needed*/
	char srtd = 1;
	int count = 0;
	DLinkedNode* curNode;
	void* tmp;

	while (count + 1 < list->count)
	{
		/*printf("Sort iter %d.\n", count);*/
		curNode = list->start->next;

		while (curNode)
		{
			if (compare(curNode->prev->content,curNode->content) > 0)
			{
				tmp = curNode->content;
				curNode->content = curNode->prev->content;
				curNode->prev->content = tmp;
				srtd = 0;
			}

			curNode = curNode->next;
		}

		if (srtd)
			break;

		count++;
	}
}

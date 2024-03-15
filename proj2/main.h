/*
 * File: main.h
 * Author: Diogo Diniz (ist1106196)
 * Description: Header for main file
*/

#ifndef MAIN_H
#define MAIN_H

#include "data.h"

/*===MACROS===*/
/*Allocation sizes*/
#define BUFFER_SIZE 65535

/*Return codes for handleCommand*/
#define CONTINUE 1
#define EXIT 0
#define NVAL -1
#define CLEAR -2

/*Turned basic testing into macros to include the return
 * without further logic.*/
#define TRY_FIND_STOP(name, storePtr, node) \
	if (!(node = findStopWithName(name, stops))) { \
		ERROR1(ERR_STOP_N_EXISTS, name);\
		free(name); \
		return;\
	}\
	free(name); storePtr = node->content;
#define TRY_NFIND_STOP(name) \
	if (findStopWithName(name, stops)) { \
		ERROR1(ERR_STOP_EXISTS, name); \
		return; \
	}
#define TRY_FIND_ROUTE(name, storePtr, node) \
	if (!(node = findRouteWithName(name, routes))) { \
		ERROR1(ERR_ROUTE_N_EXISTS, name); \
		free(name); \
		return; \
	}\
	free(name); storePtr = node->content;


/*===FUNCTION DECLARATIONS===*/
/*handleCommand - ... (Handles a given input command)
 *Command is just the first char, inputStr is the buffer pointer
 *Returns according to defined macros*/
int handleCommand(const char command, char* inputStr,
	DLinkedList* stops, DLinkedList* routes);

/*handleStopCommand - Handles 'p' commands with arguments.*/
void handleStopCommand(char* arguments, DLinkedList* stops);
/*handleRouteCommand - Handles 'c' commands with arguments.*/
void handleRouteCommand(char* arguments, DLinkedList* routes);
/*handleConnCommand - Handles 'l' commands with arguments.*/
void handleConnCommand(char* arguments, DLinkedList* stops,
	DLinkedList* routes);
/*handleIntersectCommand - Handles 'i' commands with arguments.*/
void handleIntersectCommand(DLinkedList* stops, DLinkedList* routes);
/*handleRemoveRouteCommand - Handles 'r' commands with arguments.*/
void handleRemoveRouteCommand(char* arguments, DLinkedList* routes);
/*handleRemoveStopCommand - Handles 'e' commands with arguments.*/
void handleRemoveStopCommand(char* arguments, DLinkedList* stops,
	DLinkedList* routes);

#endif

/*
 * File: main.h
 * Author: Diogo Diniz (ist1106196)
 * Description: Header for main file
 * Notes: Even though it contains a safeguard,
 * it is not intended to be used outside main.c
 * Only exception is data.c that needs constants
*/

#ifndef MAIN_H
#define MAIN_H

#include "data.h"

/*===MACROS===*/
/*Allocation sizes*/
#define MAX_STOP_COUNT 10000
#define MAX_CONNECTION_COUNT 30000
#define MAX_ROUTE_COUNT 200
#define BUFFER_SIZE BUFSIZ + 1

/*Turned basic testing into macros to include the return
 * without further logic.*/
#define TRY_FIND_STOP(name, storePtr) \
	if (!(storePtr = findStopWithName(name, stops, stopC))) { \
		ERROR1(ERR_STOP_N_EXISTS, name);\
		return;\
	}
#define TRY_NFIND_STOP(name) \
	if (findStopWithName(name, stops, stopC)) { \
		ERROR1(ERR_STOP_EXISTS, name); \
		return; \
	}
#define TRY_FIND_ROUTE(name, storePtr) \
	if (!(storePtr = findRouteWithName(name, routes, routeC))) { \
		ERROR1(ERR_ROUTE_N_EXISTS, name); \
		return; \
	}

/*===OPTION MACROS===*/
/*Uncomment for extra 't' command to dump all structures*/
/*#define DEBUG*/


/*===FUNCTION DECLARATIONS===*/
/*Handles a given input command*/
/*Command is just the first char, inputStr is the buffer pointer*/
/*Returns -1 if error, 0 if exit signal, 1 otherwise*/
int handleCommand(const char command, char* inputStr);

/*Handle command types*/
/*Takes string to the first char of the arguments*/
void handleStopCommand(char* arguments);
void handleRouteCommand(char* arguments);
void handleConnCommand(char* arguments);
/*Special case takes no arguments*/
void handleIntersectCommand();

/*Adds a stop/connection/route and returns how many free spaces are left*/
/*Mainly wrappers to the array (auto increment counters)*/
int addStop(Stop* stop);
int addConnection(Connection* conn);
int addRoute(Route* route);

#endif

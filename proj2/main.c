/*
 * File: main.c
 * Author: Diogo Diniz (ist1106196)
 * Description: Main file with entry point
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "main.h"

#include "data.h"
#include "aux.h"
#include "errors.h"


/*===LAMBDA DECLARATIONS===*/
/*Ideally these would be lambdas...*/
/*absPutStopLine - Iterator funtion to print stop lines*/
void absPutStopLine(ITER_FUNC_ARGS);
/*absPutStopLine - Iterator function to print route lines*/
void absPutRouteLine(ITER_FUNC_ARGS);
/*absPutStopLine - Iterator function to print intersection lines*/
void absPutIntersectionLine(ITER_FUNC_ARGS);
/*absPutStopLine - Iterator function to remove stops from routes*/
void absRemoveStopAndPatch(ITER_FUNC_ARGS);


/*===FUNCTION IMPLEMENTATIONS===*/
/*main - Entry point*/
int main()
{
	/*==DECLARATIONS AND ALLOCATIONS==*/
	int shouldRun = 1;
	char* inputStr;
	DLinkedList *stops, *routes;

	inputStr = malloc(sizeof(char) * (BUFFER_SIZE + 1));
	CHECK_ALLOC(inputStr);
	stops = createList();
	routes = createList();
	
	/*==EXEC LOOP==*/
	while (shouldRun)
	{
		/*Clear buffer with 0s (TODO: OPTIMIZE)*/
		memset(inputStr, 0, BUFFER_SIZE + 1);
		inputStr = fgets(inputStr, BUFFER_SIZE, stdin);
		/*Remove \n from buffer*/
		inputStr[strlen(inputStr) - 1] = '\0';

		/*In case something goes wrong, exit with code 1*/
		if ((shouldRun = handleCommand(inputStr[0], inputStr, stops, routes))
			== NVAL)
			exit(69);

		if (shouldRun == CLEAR)
		{
			destroyListElements(stops, destroyVoidStop);
			destroyListElements(routes, destroyVoidRoute);
			stops = createList();
			routes = createList();
		}
	}
	
	/*==CLEANUP==*/
	free(inputStr);
	destroyListElements(stops, destroyVoidStop);
	destroyListElements(routes, destroyVoidRoute);

	return 0;
}

/*handleCommand - Handles a command. Directly handled if with no arguments,
  calls the relevant function if arguments are given.*/
int handleCommand(const char command, char* inputStr,
	DLinkedList* stops, DLinkedList* routes)
{
	int argLen = strlen(inputStr);
	switch (command)
	{
		case 'q':
			return EXIT;
		case 'p':
			if (argLen > 2) /*Has arguments*/
				handleStopCommand(inputStr+2, stops);
			else /*No arguments*/
				iterateListContents(stops, 0, &absPutStopLine,
					(void**)&routes, 1);
			break;
		case 'c':
			if (argLen > 2) /*Has arguments*/
				handleRouteCommand(inputStr+2, routes);
			else /*No arguments*/
				iterateListContents(routes, 0, &absPutRouteLine, NULL, 0);
			break;
		case 'l':
			if (argLen > 2) /*Has arguments*/
				handleConnCommand(inputStr+2, stops, routes);
			break; /*Ignore otherwise*/
		case 'i':
			handleIntersectCommand(stops, routes); /*Ignore possible arguments*/
			break;
		case 'a':
			return CLEAR;
		case 'r':
			if (argLen > 2)
				handleRemoveRouteCommand(inputStr+2, routes);
			break; /*Ignore otherwise*/
		case 'e':
			if (argLen > 2)
				handleRemoveStopCommand(inputStr+2, stops, routes);
			break; /*Ignore otherwise*/
		default:
			return NVAL;
	}
	return CONTINUE;
}

/*handleStopCommand - Reads arguments one at a time and interprets at the end*/
void handleStopCommand(char* arguments, DLinkedList* stops)
{
	int argN = 0;
	char* name = NULL; double lat = 0, lon = 0;
	char* curSrc = arguments;
	DLinkedNode* node;
	Stop* stop;

	while (*curSrc)
	{
		switch (argN)
		{
			case 0: /*Looking for name*/
				name = copyName(curSrc, &curSrc);
				break;

			case 1: /*Looking for lat*/
				lat = copyFloat(curSrc, &curSrc);
				break;

			case 2: /*Looking for lon*/
				lon = copyFloat(curSrc, &curSrc);
				break;

			default:
				ERROR0("Invalid argument count for 'p'.");
				exit(-1);
		}
		argN++;
	}

	if (argN == 1) /*Search and print lat lon*/
	{
		TRY_FIND_STOP(name, stop, node);
		printf("%16.12f %16.12f\n", stop->lat, stop->lon);
	}
	else if (argN == 3) /*Add*/
	{
		TRY_NFIND_STOP(name);

		appendContentToList(stops, createStop(lat, lon, name));
	}
}
/*handleRouteCommand- Reads arguments one at a time and interprets at the end*/
void handleRouteCommand(char* arguments, DLinkedList* routes)
{
	int argN = 0;
	char* name = NULL; char inv = 0;
	char* curSrc = arguments;
	DLinkedNode* node;
	Route* route = NULL;
	
	while (*curSrc)
	{
		switch (argN)
		{
			case 0: /*Looking for name*/
				name = copyNameNoQuotes(curSrc, &curSrc);
				node = findRouteWithName(name, routes);
				route = node ? (Route*)node->content : NULL;
				break;

			case 1: /*Looking for inv*/
				if (!node)
					return;
				if (!isInvert(curSrc))
				{
					ERROR0(ERR_INV_SORT);
					return;
				}
				inv = 1;
				*curSrc = 0; /*Just set it to zero to break*/
				break;

			default:
				ERROR0("Invalid argument count for 'c'.\n");
				exit(-1);
		}

		argN++;
	}

	if (route) /*If route exists print*/
	{
		/*Print routes (reversed if needed)*/
		if (inv)
			putRouteLineStops_rev(route);
		else
			putRouteLineStops(route);

		free(name);
	}
	else if (argN == 1) /*Else add new*/
	{
		appendContentToList(routes, createRoute(name));
	}
}
/*handleConnCommand - Reads arguments one at a time and interprets at the end*/
void handleConnCommand(char* arguments, DLinkedList* stops,
	DLinkedList* routes)
{
	int argN = 0;
	char* name = "", *startName = "", *endName = "";
	float cost = 0, duration = 0;
	char* curSrc = arguments;
	DLinkedNode* node = NULL;
	Stop *startS = NULL, *endS = NULL;
	Route* route = NULL;
	Connection* conn = NULL;
	
	while (*curSrc)
	{
		switch (argN)
		{
			case 0: /*Looking for name*/
				name = copyNameNoQuotes(curSrc, &curSrc);
				break;

			case 1: /*Looking for first stop name*/
				startName = copyName(curSrc, &curSrc);
				break;

			case 2: /*Looking for last stop name*/
				endName = copyName(curSrc, &curSrc);
				break;

			case 3: /*Looking for cost*/
				cost = copyFloat(curSrc, &curSrc);
				break;

			case 4: /*Looking for duration*/
				duration = copyFloat(curSrc, &curSrc);
				break;

			default:
				ERROR0("Invalid argument count for 'l'.");
				exit(-1);
		}

		argN++;
	}

	if (argN != 5)
		return;

	TRY_FIND_ROUTE(name, route, node);
	TRY_FIND_STOP(startName, startS, node); 
	TRY_FIND_STOP(endName, endS, node);

	conn = createConnection(startS, endS, cost, duration);

	if (cost >= 0 && duration >= 0)
	{
		if (!route_addConnection(route, conn))
		{
			destroyConnection(conn);
			ERROR0(ERR_LINK_N_ASSOC);
			return;
		}
	}
	else
	{
		destroyConnection(conn);
		ERROR0(ERR_NEG_VAL);
		return;
	}
}
/*handleIntersectCommand - Calls an iterator to check all stops for intersect*/
void handleIntersectCommand(DLinkedList* stops, DLinkedList* routes)
{
	/*Route listing must be ordered*/
	iterateListContents(stops, 0, &absPutIntersectionLine, (void**)&routes, 1);
}
/*handleRemoveRouteCommand - Searches and removes the route in the arguments*/
void handleRemoveRouteCommand(char* arguments, DLinkedList* routes)
{
	int argN = 0;
	char* name = "";
	char* curSrc = arguments;
	DLinkedNode* node = NULL;
	Route* route = NULL;

	while (*curSrc)
	{
		switch (argN)
		{
			case 0: /*Looking for name*/
				name = copyNameNoQuotes(curSrc, &curSrc);
				break;

			default:
				ERROR0("Invalid argument count for 'r'.");
				exit(-1);
		}

		argN++;
	}

	TRY_FIND_ROUTE(name, route, node);

	destroyRoute(route);
	removeNodeFromList(routes, node);
}
/*handleRemoveStopCommand - Searches and removes the stop in the arguments*/
void handleRemoveStopCommand(char* arguments, DLinkedList* stops,
	DLinkedList* routes)
{
	int argN = 0;
	char* name = "";
	char* curSrc = arguments;
	DLinkedNode* node = NULL;
	Stop* stop = NULL;

	while (*curSrc)
	{
		switch (argN)
		{
			case 0: /*Looking for name*/
				name = copyName(curSrc, &curSrc);
				break;

			default:
				ERROR0("Invalid argument count for 'e'.");
				exit(-1);
		}

		argN++;
	}

	TRY_FIND_STOP(name, stop, node);

	iterateListContents(routes, 0, &absRemoveStopAndPatch, (void**)&stop, 1);

	destroyStop(stop);
	removeNodeFromList(stops, node);
}


/*===LAMBDAS===*/
/*absPutStopLine - ...*/
void absPutStopLine(ITER_FUNC_ARGS)
{
	ITER_FUNC_CHECK_ARGC(1);

	putStopLine((Stop*)content, args[0]);
}
/*absPutRouteLine - ...*/
void absPutRouteLine(ITER_FUNC_ARGS)
{
	ITER_FUNC_DISCARD_ARGS();

	putRouteLine((Route*)content);
}
/*absPutIntersectionLine - ...*/
void absPutIntersectionLine(ITER_FUNC_ARGS)
{
	ITER_FUNC_CHECK_ARGC(1);

	putIntersectionLine((Stop*)content, args[0]);
}
/*absRemoveStopAndPatch - ...*/
void absRemoveStopAndPatch(ITER_FUNC_ARGS)
{
	ITER_FUNC_CHECK_ARGC(1);

	removeStopAndPatch((Route*)content, (Stop*)args[0]);
}

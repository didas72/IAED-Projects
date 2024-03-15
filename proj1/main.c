/*
 * File: main.c
 * Author: Diogo Diniz (ist1106196)
 * Description: Main file with entry point
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"

#include "data.h"
#include "aux.h"
#include "errors.h"
#include "dbg.h"


/*===GLOBAL VAR DECLARATIONS===*/
/*Storage arrays and current count*/
int stopC, connC, routeC;
Stop** stops;
Connection** conns;
Route** routes;


/*===ENTRY POINT===*/
int main()
{
	/*==DECLARATIONS AND ALLOCATIONS==*/
	int shouldRun = 1;
	char* inputStr = malloc(sizeof(char) * BUFFER_SIZE);

	stops = malloc(sizeof(Stop*) * MAX_STOP_COUNT); stopC = 0;
	conns = malloc(sizeof(Connection*) * MAX_CONNECTION_COUNT); connC = 0;
	routes = malloc(sizeof(Route*) * MAX_ROUTE_COUNT); routeC = 0;


	/*==EXEC LOOP==*/
	while (shouldRun)
	{
		/*Clear buffer with 0s (TODO: OPTIMIZE)*/
		memset(inputStr, 0, BUFFER_SIZE);
		/*Assignement is needless, but -Werror is a thing...*/
		inputStr = fgets(inputStr, BUFSIZ, stdin);
		/*Remove \n from buffer*/
		inputStr[strlen(inputStr) - 1] = '\0';

		/*In case something goes wrong, exit with code 1*/
		if ((shouldRun = handleCommand(inputStr[0], inputStr)) == -1)
			exit(1);
	}
	

	/*==CLEANUP==*/
	/*Shouldn't be needed but might as well do it*/
	free(inputStr);
	/*TODO: (Maybe) Iter clean each element*/
	free(stops);
	free(conns);
	free(routes);

	return 0;
}


/*===FUNCTION IMPLEMENTATIONS===*/
int handleCommand(const char command, char* inputStr)
{
	int agrLen = strlen(inputStr), i;

	switch (command)
	{
		case 'q':
			return 0;

		case 'p':
			if (agrLen > 2) /*Has arguments*/
				handleStopCommand(inputStr+2);
			else /*No arguments*/
				for (i = 0; i < stopC; i++)
					putStopLine(stops[i], routes, routeC);
			break;

		case 'c':
			if (agrLen > 2) /*Has arguments*/
				handleRouteCommand(inputStr+2);
			else /*No arguments*/
				for (i = 0; i < routeC; i++)
					putRouteLine(routes[i]);
			break;

		case 'l':
			if (agrLen > 2) /*Has arguments*/
				handleConnCommand(inputStr+2);
			break; /*Ignore otherwise*/

		case 'i':
			handleIntersectCommand(); /*Ignore possible arguments*/
			break;

	#ifdef DEBUG
		/*This can save some time, enable in main.h*/
		case 't':
			DumpStructures(stops, stopC, conns, connC, routes, routeC);
			break;
	
		case ' ':
		case '\n':
		case '\t':
			/*Ignore whitespaces (easier debugging)*/
			break;
	#endif

		default:
			return -1;
	}

	return 1;
}


void handleStopCommand(char* arguments)
{
	int argN = 0;
	char* name = NULL; double lat = 0, lon = 0;
	char* curSrc = arguments;
	Stop* stop;

	while (*curSrc)
	{
		switch (argN)
		{
			case 0: /*Looking for name*/
				name = copyName(curSrc, STOP_NAME_MAX_LEN, &curSrc);
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
		/*TODO: Fix memory leak with names*/
		TRY_FIND_STOP(name, stop);
		printf("%16.12f %16.12f\n", stop->lat, stop->lon);
	}
	else if (argN == 3) /*Add*/
	{
		TRY_NFIND_STOP(name);

		addStop(createStop(lat, lon, name));
	}
}
void handleRouteCommand(char* arguments)
{
	int argN = 0;
	char* name = NULL; char inv = 0;
	char* curSrc = arguments;
	Route* route = NULL;
	
	while (*curSrc)
	{
		switch (argN)
		{
			case 0: /*Looking for name*/
				name = copyNameNoQuotes(curSrc, ROUTE_NAME_MAX_LEN, &curSrc);
				route = findRouteWithName(name, routes, routeC);
				break;

			case 1: /*Looking for inv*/
				if (!route)
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
	}
	else if (argN == 1) /*Else add new*/
	{
		if (!addRoute(createRoute(name)))
			ERROR0(ERR_LINK_N_ASSOC);
	}
}
void handleConnCommand(char* arguments)
{
	int argN = 0;
	char* name = "", *startName = "", *endName = "";
	float cost = 0, duration = 0;
	char* curSrc = arguments;
	Stop* startS, *endS;
	Route* route;
	Connection* conn;
	
	while (*curSrc)
	{
		switch (argN)
		{
			case 0: /*Looking for name*/
				name = copyNameNoQuotes(curSrc, ROUTE_NAME_MAX_LEN, &curSrc);
				break;

			case 1: /*Looking for first stop name*/
				startName = copyName(curSrc, STOP_NAME_MAX_LEN, &curSrc);
				break;

			case 2: /*Looking for last stop name*/
				endName = copyName(curSrc, STOP_NAME_MAX_LEN, &curSrc);
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

	TRY_FIND_ROUTE(name, route);
	TRY_FIND_STOP(startName, startS); 
	TRY_FIND_STOP(endName, endS);

	conn = createConnection(startS, endS, cost, duration);

	if (cost >= 0 && duration >= 0)
	{
		if (!route_addConnection(route, conn))
		{
			/*Let's try to avoid memory leaks*/
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

	addConnection(conn);
}
void handleIntersectCommand()
{
	/*Route listing must be ordered*/
	int i;

	for (i = 0; i < stopC; i++)
		putIntersectionLine(stops[i], routes, routeC);
}


int addStop(Stop* stop)
{
	stops[stopC++] = stop;
	return MAX_STOP_COUNT - stopC;
}
int addConnection(Connection* conn)
{
	conns[connC++] = conn;
	return MAX_CONNECTION_COUNT - connC;
}
int addRoute(Route* route)
{
	routes[routeC++] = route;
	return MAX_ROUTE_COUNT - routeC + 1;
}

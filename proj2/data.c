/*
 * File: data.c
 * Author: Diogo Diniz (ist1106196)
 * Description: Implementation of functions in data.h
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "data.h"
#include "errors.h"


/*===FUNCTION DECLARATIONS===*/
/*Don't make available*/
char routeContainsStop(ITER_FUNC_ARGS);



/*===LAMBDA DECLARATIONS===*/
/*Ideally these would be lambdas or nested but old C...*/
/*putEndStop - Iterator funtion to print the last stop of a connection.
  Takes no arguments*/
void putEndStop(ITER_FUNC_ARGS);
/*putStartStop - Iterator funtion to print the first stop of a connection.
  Takes no arguments*/
void putStartStop(ITER_FUNC_ARGS);
/*putRouteName - Iterator funtion to print the name of a route.
  Takes no arguments*/
void putRouteName(ITER_FUNC_ARGS);
/*getConnCost - Iterator funtion to get the cost of a connection.
  Takes no arguments*/
float getConnCost(ITER_FUNC_ARGS);
/*getConnDuration - Iterator funtion to get the duration of a connection.
  Takes no arguments*/
float getConnDuration(ITER_FUNC_ARGS);
/*containsStop - Iterator function to check if the iterated connection 
  contains the given stop.
  Takes (Stop* stop)*/
char containsStop(ITER_FUNC_ARGS);
/*stopHasName - Iterator function to check if the iterated stop
  has the given name.
  Takes (char* name)*/
char stopHasName(ITER_FUNC_ARGS);
/*routeHasName - Iterator function to check if the iterated route
  has the given name.
  Takes (char* name)*/
char routeHasName(ITER_FUNC_ARGS);
/*addIfContainsStop - Iterator function to add the iterated route
  to the given list if it contains the given stop.
  Takes (DLinkedList* list, Stop* stop)*/
void addIfContainsStop(ITER_FUNC_ARGS);
/*cmpRouteName - Modified iterator function to compare the given
  route names.
  Used to sort routes alphabetically.*/
int cmpRouteName(void* routeA, void* routeB);


/*===LOCAL DECLARATIONS===*/
/*removeStopAndPatchInterior - Same as removeStopAndPatch but for
  stops in the middle of a route*/
void removeStopAndPatchInterior(Route* route, Stop* stop);

/*===FUNCTION IMPLEMENTATIONS===*/
/*createStop - Simple allocation with validation and assignement*/
Stop* createStop(const double lat, const double lon, char* name)
{
	Stop* ret = malloc(sizeof(Stop));
	CHECK_ALLOC(ret);
	ret->lat = lat;
	ret->lon = lon;
	ret->name = name;
	return ret;
}
/*createConnection - Simple allocation with validation and assignement*/
Connection* createConnection(Stop* start, Stop* end,
	const float cost, const float duration)
{
	Connection* ret = malloc(sizeof(Connection));
	CHECK_ALLOC(ret);
	ret->start = start;
	ret->end = end;
	ret->cost = cost;
	ret->duration = duration;
	return ret;
}
/*createRoute - Simple allocation with validation and assignement*/
Route* createRoute(char* name)
{
	Route* ret = malloc(sizeof(Route));
	CHECK_ALLOC(ret);
	ret->name = name;
	ret->connections = createList();
	return ret;
}


/*destroyStop - Simple frees*/
void destroyStop(Stop* stop)
{
	free(stop->name);
	free(stop);
}
/*destroyVoidStop - Simple pointer cast and destroyStop call*/
void destroyVoidStop(void* stop)
{
	destroyStop((Stop*)stop);
}
/*destroyConnection - Simple frees*/
void destroyConnection(Connection* conn)
{		
	free(conn);
}
/*destroyVoidConnection - Simple pointer cast and destroyConnection call*/
void destroyVoidConnection(void* conn)
{
	destroyConnection((Connection*)conn);
}
/*destroyRoute - Simple frees*/
void destroyRoute(Route* route)
{
	destroyListElements(route->connections, &destroyVoidConnection);
	free(route->name);
	free(route);
}
/*destroyVoidRoute - Simple pointer cast and destroyRoute call*/
void destroyVoidRoute(void* route)
{
	destroyRoute((Route*)route);
}


/*route_addConnection - Compares start/end stops with route start/end
  and adds stop if possible. Prints an error if not possible.*/
char route_addConnection(Route* route, Connection* conn)
{
	if (!route->connections->count)
	{
		appendContentToList(route->connections, conn);
	}
	else if (conn->start ==
		((Connection*)route->connections->end->content)->end
		|| route->connections->count == 0)
	{
		appendContentToList(route->connections, conn);
	}
	else if (conn->end ==
		((Connection*)route->connections->start->content)->start)
	{
		prependContentToList(route->connections, conn);
	}
	else 
		return 0;

	return 1;
}


/*putStopLine - Simple printf with formatting*/
void putStopLine(const Stop* stop, DLinkedList* routes)
{
	int routeNum = countRoutesWithStop(stop, routes);
	printf("%s: %16.12f %16.12f %d\n", stop->name, stop->lat,
		stop->lon, routeNum);
}
/*putRouteLine - Simple printf with formatting*/
void putRouteLine(const Route* route)
{
	int stopN = countRouteStops(route);
	float costT = getTotalCost(route), durationT = getTotalDuration(route);
	Connection* firstC = route->connections->start ?
		route->connections->start->content : NULL;
	Connection* lastC = route->connections->end ?
		route->connections->end->content : NULL;

	/*Put name first*/
	printf("%s ", route->name);

	/*Print stop names if applicable*/
	if (firstC)
		printf("%s ", firstC->start->name);
	if (lastC)
		printf("%s ", lastC->end->name);

	printf("%d %.2f %.2f\n",
		stopN, costT, durationT);
}
/*putRouteLineStops - Iterates stops and print names*/
void putRouteLineStops(const Route* route)
{
	if (!route->connections->start)
		return;

	if (route->connections->start)
		printf("%s", 
			((Connection*)route->connections->start->content)
			->start->name);

	iterateListContents(route->connections, 0, &putEndStop, NULL, 0);
	
	putchar('\n');
}
/*putRouteLineStops_rev - Iterates stops and print names in reverse order*/
void putRouteLineStops_rev(const Route* route)
{
	if (!route->connections->start)
		return;

	if (route->connections->end)
		printf("%s", 
			((Connection*)route->connections->end->content)
			->end->name);

	iterateListContents(route->connections, 1, &putStartStop, NULL, 0);
	
	putchar('\n');
}


/*findStopWithName - Calls iterator to find first name match*/
DLinkedNode* findStopWithName(const char* name,
	DLinkedList* stops)
{
	return findFirstListContent(stops, &routeHasName, (void**)&name, 1);
}
/*findRouteWithName - Calls iterator to find first name match*/
DLinkedNode* findRouteWithName(const char* name,
	DLinkedList* routes)
{
	return findFirstListContent(routes, &routeHasName, (void**)&name, 1);
}


/*countRoutesWithStop - Calls iterator to count all routes with the stop*/
int countRoutesWithStop(const Stop* stop,
	DLinkedList* routes)
{
	return iterateCountListContents(routes, &routeContainsStop, (void**)&stop, 1);
}
/*countRouteStops - Direct list struct access to determine stop count*/
int countRouteStops(const Route* route)
{
	if (route->connections->count)
		return route->connections->count + 1;

	return 0;
}
/*putIntersectionLine - Finds all routes containing the given stop, sorts
  them by name and prints.*/
void putIntersectionLine(const Stop* stop,
	DLinkedList* routes)
{
	DLinkedList* iRoutes = createList();
	void** args = malloc(2 * sizeof(void*));
	CHECK_ALLOC(args);

	args[0] = (void*)iRoutes;
	args[1] = (void*)stop;

	iterateListContents(routes, 0, &addIfContainsStop,
		args, 2);

	free(args);

	if (iRoutes->count < 2) /*Ignore if no intersection*/
	{
		destroyList(iRoutes);
		return;
	}

	sortListContents(iRoutes, &cmpRouteName);

	printf("%s %d:", stop->name, iRoutes->count);

	iterateListContents(iRoutes, 0, &putRouteName, NULL, 0);

	putchar('\n');

	destroyList(iRoutes);
}

/*getTotalCost - Calls iterator to sum all connection costs*/
float getTotalCost(const Route* route)
{
	return iterateSumListContents(route->connections, &getConnCost, NULL, 0);
}
/*getTotalDuration - Calls iterator to sum all connection durations*/
float getTotalDuration(const Route* route)
{
	return iterateSumListContents(route->connections, &getConnDuration, NULL, 0);
}

/*routeContainsStop - Calls iterator to check if any stops matches name*/
char routeContainsStop(ITER_FUNC_ARGS)
{
	ITER_FUNC_CHECK_ARGC(1);

	return iterateAnyListContents(((Route*)content)->connections, &containsStop,
		(void**)&args[0], 1);
}
/*removeStopAndPatch - Repeatedly removes the first stop from a route
  until it is not the given stop and calls removeStopAndPatchInterior*/
void removeStopAndPatch(Route* route, Stop* stop)
{
	DLinkedNode *curNode, *tmp;
	Connection *curConn;
	char repeat = 1;

	if (!route->connections->count) /*Skip empty routes*/
		return;

	/*Exceptionally iterate here*/
	curNode = route->connections->start;

	/*Clear tips*/
	while (repeat)
	{
		repeat = 0;
		curConn = curNode->content;

		if (curConn->start == stop) /*First is stop*/
		{
			repeat = 1;
			tmp = curNode->next;
			removeNodeFromList(route->connections, curNode);
			curNode = tmp;
			destroyConnection(curConn);
			/*No need to do anything else, cost adjusts itself*/
			if (!tmp)
				repeat = 0;
		}
	}

	removeStopAndPatchInterior(route, stop);
}
/*removeStopAndPatchInterior - Iterates route stops and
  removes them if matching, creating new connections when needed.*/
void removeStopAndPatchInterior(Route* route, Stop* stop)
{
	DLinkedNode *curNode = route->connections->start, *tmp;
	Connection *curConn, *nextConn;

	/*Interior and last*/
	while (curNode)
	{
		curConn = curNode->content;

		if (curConn->end == stop) /*End case is here*/
		{
			if (curNode->next)
			{
				/*Adjust next conn and remove if not last*/
				nextConn = curNode->next->content;

				nextConn->cost += curConn->cost;
				nextConn->duration += curConn->duration;
				nextConn->start = curConn->start;

				tmp = curNode->next;

				removeNodeFromList(route->connections, curNode);
				curNode = tmp;
				destroyConnection(curConn);
				continue;
			}
			else
			{
				/*Just remove if last conn*/
				removeNodeFromList(route->connections, curNode);
				curNode = NULL;
				destroyConnection(curConn);
				continue;
				/*No need to do anything else, cost adjusts itself*/
			}
		}

		curNode = curNode->next;
	}
}

/*===LAMBDAS===*/
/*putEndStop - Simple printf with formatting*/
void putEndStop(ITER_FUNC_ARGS)
{
	ITER_FUNC_DISCARD_ARGS();
	printf(", %s", ((Connection*)content)->end->name);
}
/*putStartStop - Simple printf with formatting*/
void putStartStop(ITER_FUNC_ARGS)
{
	ITER_FUNC_DISCARD_ARGS();
	printf(", %s", ((Connection*)content)->start->name);
}
/*putRouteName - Simple printf with formatting*/
void putRouteName(ITER_FUNC_ARGS)
{
	ITER_FUNC_DISCARD_ARGS();
	printf(" %s", ((Route*)content)->name);
}
/*getConnCost - Simple connection struct access*/
float getConnCost(ITER_FUNC_ARGS)
{
	ITER_FUNC_DISCARD_ARGS();
	return ((Connection*)content)->cost;
}
/*getConnDuration - Simple connection struct access*/
float getConnDuration(ITER_FUNC_ARGS)
{
	ITER_FUNC_DISCARD_ARGS();
	return ((Connection*)content)->duration;
}
/*containsStop - Simple connection struct access*/
char containsStop(ITER_FUNC_ARGS)
{
	ITER_FUNC_CHECK_ARGC(1);

	return ((Connection*)content)->start == args[0]
		|| ((Connection*)content)->end == args[0];
}
/*stopHasName - Simple strcmp to match names*/
char stopHasName(ITER_FUNC_ARGS)
{
	ITER_FUNC_CHECK_ARGC(1);

	return strcmp(((Stop*)content)->name, args[0]) == 0;
}
/*routeHasName - Simple strcmp to match names*/
char routeHasName(ITER_FUNC_ARGS)
{
	ITER_FUNC_CHECK_ARGC(1);

	return strcmp(((Route*)content)->name, args[0]) == 0;
}
/*addIfContainsStop - Verifies if route contains stop and adds to list*/
void addIfContainsStop(ITER_FUNC_ARGS)
{
	ITER_FUNC_CHECK_ARGC(2);

	if (routeContainsStop(content, (void**)&args[1], 1))
		appendContentToList((DLinkedList*)args[0], content);
}
/*cmpRouteName - Simple strcmp wrapper*/
int cmpRouteName(void* routeA, void* routeB)
{
	return strcmp(((Route*)routeA)->name,
		((Route*)routeB)->name);
}

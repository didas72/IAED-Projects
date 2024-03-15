/*
 * File: data.c
 * Author: Diogo Diniz (ist1106196)
 * Description: Implementation of functions in data.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
/*Exceptionally include for constants*/
#include "main.h"


/*===FUNCTION DECLARATIONS===*/
/*Don't make available*/
char routeContainsStop(Route* route, const Stop* stop);



/*===FUNCTION IMPLEMENTATIONS===*/
Stop* createStop(const double lat, const double lon, char* name)
{
	Stop* ret = malloc(sizeof(Stop));
	ret->lat = lat;
	ret->lon = lon;
	ret->name = name;
	return ret;
}
Connection* createConnection(Stop* start, Stop* end,
	const float cost, const float duration)
{
	Connection* ret = malloc(sizeof(Connection));
	ret->start = start;
	ret->end = end;
	ret->cost = cost;
	ret->duration = duration;
	ret->prev = ret->next = NULL;
	return ret;
}
Route* createRoute(char* name)
{
	Route* ret = malloc(sizeof(Route));
	ret->name = name;
	ret->last = ret->first = NULL;
	return ret;
}


void destroyStop(Stop* stop)
{
	free(stop->name);
	free(stop);
}
void destroyConnection(Connection* conn)
{
	/*Preserve dlinked list*/
	if (conn->prev)
		conn->prev->next = conn->next;
	if (conn->next)
		conn->next->prev = conn->prev;
		
	free(conn);
}
void destoyRoute(Route* route)
{
	/*Don't attempt to free dlinked list*/
	free(route);
}


char route_addConnection(Route* route, Connection* conn)
{
	if (!(route->first)) /*Empty route*/
	{
		route->first = route->last = conn;
	}
	else if (conn->start == route->last->end) /*Conn start = end of route*/
	{
		conn->prev = route->last;
		conn->prev->next = conn;
		route->last = conn;
	}
	else if (conn->end == route->first->start) /*Conn end = start of route*/
	{
		conn->next = route->first;
		conn->next->prev = conn;
		route->first = conn;
	}
	else 
		return 0;

	return 1;
}


void putStopLine(const Stop* stop, Route** routes, const int routeC)
{
	int routeNum = countRoutesWithStop(stop, routes, routeC);
	printf("%s: %16.12f %16.12f %d\n", stop->name, stop->lat, stop->lon, routeNum);
}
void putRouteLine(const Route* route)
{
	int stopN = countRouteStops(route);
	float costT = getTotalCost(route), durationT = getTotalDuration(route);
	Connection* firstC = (route->first);
	Connection* lastC = (route->last);

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
void putRouteLineStops(const Route* route)
{
	Connection* conn;

	if (!(conn = route->first))
	{
		#ifdef PRINT_EMPTY_ROUTES
		printf("\n");
		#endif
		return;
	}

	printf("%s, %s", conn->start->name, conn->end->name);

	while ((conn = conn->next))
		printf(", %s", conn->end->name);
	
	putchar('\n');
}
void putRouteLineStops_rev(const Route* route)
{
	Connection* conn;

	if (!(conn = route->last))
	{
		#ifdef PRINT_EMPTY_ROUTES
		printf("\n");
		#endif
		return;
	}

	printf("%s, %s", conn->end->name, conn->start->name);

	while ((conn = conn->prev))
		printf(", %s", conn->start->name);
	
	putchar('\n');
}


Stop* findStopWithName(const char* name,
	Stop** stops, const int stopC)
{
	int i;

	for (i = 0; i < stopC; i++)
	{
		if (!strcmp(stops[i]->name, name))
			return stops[i];
	}

	return NULL;
}
Route* findRouteWithName(const char* name,
	Route** routes, const int routeC)
{
	int i;

	for (i = 0; i < routeC; i++)
	{
		if (!strcmp(routes[i]->name, name))
			return routes[i];
	}

	return NULL;
}


int countRoutesWithStop(const Stop* stop,
	Route** routes, const int routeC)
{
	int i, counter = 0;

	for (i = 0; i < routeC; i++)
		counter += routeContainsStop(routes[i], stop);

	return counter;
}
int countRouteStops(const Route* route)
{
	int counter;
	Connection* conn = route->first;

	if (!conn)
		return 0;

	counter = 2;

	while ((conn = conn->next))
		counter++;

	return counter;
}


void putIntersectionLine(const Stop* stop,
	Route** routes, const int routeC)
{
	/*l prefix for local, assume all routes can intersect*/
	Route* lRoutes[MAX_ROUTE_COUNT], *tmp;
	int i, j, count = 0;

	for (i = 0; i < routeC; i++)
	{
		/*Add all intersecting routes to array*/
		if (routeContainsStop(routes[i], stop))
			lRoutes[count++] = routes[i];
	}

	if (count < 2) /*Ignore if no intersection*/
		return;

	/*Sort array (bubble sort, 200 routes doesn't need anything fancy)*/
	for (j = 0; j < count - 1; j++)
	{
		/*Up sort, don't need to sort top most elements*/
		for (i = 1; i < count - j; i++)
		{
			if (strcmp(lRoutes[i-1]->name, lRoutes[i]->name) > 0)
			{
				tmp = lRoutes[i-1];
				lRoutes[i-1] = lRoutes[i];
				lRoutes[i] = tmp;
			}
		}
	}

	printf("%s %d: %s", stop->name, count, lRoutes[0]->name);

	for (i = 1; i < count; i++)
		printf(" %s", lRoutes[i]->name);

	putchar('\n');
}


float getTotalCost(const Route* route)
{
	float total;
	Connection* conn = route->first;

	if (!conn)
		return .0;

	total = conn->cost;

	while ((conn = conn->next))
		total += conn->cost;

	return total;
}
float getTotalDuration(const Route* route)
{
	float total;
	Connection* conn = route->first;

	if (!conn)
		return .0;

	total = conn->duration;

	while ((conn = conn->next))
		total += conn->duration;

	return total;
}


char routeContainsStop(Route* route, const Stop* stop)
{
	Connection* conn = route->first;

	if (!conn)
		return 0;

	if (stop == conn->start || stop == conn->end)
		return 1;

	while ((conn = conn->next))
	{
		if (stop == conn->end)
			return 1;
	}

	return 0;
}

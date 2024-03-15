/*
 * File: dbg.c
 * Author: Diogo Diniz (ist1106196)
 * Description: Implementation of functions in dbg.h
*/

#include <stdio.h>

#include "data.h"
#include "aux.h"
#include "errors.h"
#include "dbg.h"


/*===MACROS===*/
#define ptr(a) (void*)a


/*===FUNCTION DECLARATIONS===*/
/*Don't make available*/
int findIndexOf(void* ptr, void** arr, int arrC);


/*===FUNCTION IMPLEMENTATIONS===*/
void DumpStructures(Stop** stops, const int stopC,
	Connection** conns, const int connC,
	Route** routes, const int routeC)
{
	int i, pI, nI; char *fName, *lName;
	
	printf("\n====DBG: STRUCTURE DUMP====\n");



	/*Stop dump*/
	printf("\n\t==%d stops present (%ldkB) at base @%p==\n", stopC, (stopC * sizeof(Stop)) >> 10, ptr(stops));

	for (i = 0; i < stopC; i++)
		printf("Stop %5d @%p: { name='%-20s'; lat=%16.12f; lon=%16.12f }\n",
			i, ptr(stops[i]), stops[i]->name, stops[i]->lat, stops[i]->lon);



	/*Connection dump*/
	printf("\n\t==%d connections present (%ldkB) at base @%p==\n", connC, (connC * sizeof(Connection)) >> 10, ptr(conns));

	for (i = 0; i < connC; i++)
	{
		fName = conns[i]->start ? conns[i]->start->name : NULL;
		lName = conns[i]->end ? conns[i]->end->name : NULL;

		pI = findIndexOf(conns[i]->prev, (void**)conns, connC);
		nI = findIndexOf(conns[i]->next, (void**)conns, connC);

		printf("Conn %5d @%p: { start=@%p (name='%-15s'); end=@%p (name='%-15s'); prev=@%p (%d); next=@%p (%d); cost=%.2f; duration=%.2f }\n",
			i, ptr(conns[i]), ptr(conns[i]->start), fName, ptr(conns[i]->end), lName,
			ptr(conns[i]->prev), pI, ptr(conns[i]->next), nI, conns[i]->cost, conns[i]->duration);
	}



	/*Raw route dump*/
	printf("\n\t==%d routes present (%ldkB) at base @%p==\n", routeC, (stopC * sizeof(Route)) >> 10, ptr(routes));

	for (i = 0; i < routeC; i++)
	{
		pI = findIndexOf(routes[i]->first, (void**)conns, connC);
		nI = findIndexOf(routes[i]->last, (void**)conns, connC);

		printf("Route %3d @%p: { name='%-20s'; first=@%p (%d); last=@%p (%d) }\n",
			i, ptr(routes[i]), routes[i]->name, ptr(routes[i]->first), pI, ptr(routes[i]->last), nI);
	}

	printf("\n====DBG: END STRUCTURE DUMP====\n\n");
}

int findIndexOf(void* ptr, void** arr, int arrC)
{
	int i = 0;

	if (ptr == NULL)
		return -69;

	for (; i < arrC; i++)
		if (arr[i] == ptr)
			return i;
	
	return -1;
}

/*
 * File: data.h
 * Author: Diogo Diniz (ist1106196)
 * Description: Data structures and related functions header
*/

#ifndef DATA_H
#define DATA_H

#include "collections.h"


/*===DATA STRUCTURES===*/
typedef struct Connection Connection;

typedef struct
{
	char* name; /*0-3*/
	double lat, lon; /*4-7;8-11*/
} Stop;

struct Connection
{
	Stop* start, *end; /*0-3;4-7*/
	float cost, duration; /*16-19;20-24*/
};

typedef struct
{
	char* name;
	DLinkedList* connections;
} Route;


/*===FUNCTION DECLARATIONS===*/
/*=Constructors=*/
/*createStop - Allocates and initializes a Stop with the given data*/
Stop* createStop(const double lat, const double lon, char* name);
/*createConnection - Allocates and initializes a Connection
  with the given data*/
Connection* createConnection(Stop* start, Stop* end,
	const float cost, const float duration);
/*createRoute - Allocates and initializes a Route with the given data*/
Route* createRoute(char* name);

/*=Destructors=*/
/*destroyStop - Frees a Stop structure and associated resources*/
void destroyStop(Stop* stop);
/*destroyVoidStop - Calls destroyStop after casting the pointer*/
void destroyVoidStop(void* stop);
/*destroyConnection - Frees a Connection structure and associated resources*/
void destroyConnection(Connection* conn);
/*destroyVoidConnection - Calls destroyConnection after casting the pointer*/
void destroyVoidConnection(void* conn);
/*destroyRoute - Frees a Route structure and associated resources*/
void destroyRoute(Route* route);
/*destroyVoidRoute - Calls destroyRoute after casting the pointer*/
void destroyVoidRoute(void* route);

/*route_addConnection - Attempts to add connection to route.
  Returns non-zero if successful*/
char route_addConnection(Route* route, Connection* conn);

/*putStopLine - Prints formatted Stop line to stdout*/
void putStopLine(const Stop* stop, DLinkedList* routes);
/*putRouteLine - Prints formatted Route line to stdout*/
void putRouteLine(const Route* route); 
/*putRouteLineStops - Prints Route stops to stdout*/
void putRouteLineStops(const Route* route);
/*putRouteLineStops_rev - Prints Route stops to stdout in reverse order*/
void putRouteLineStops_rev(const Route* route);
/*putIntersectionLine - Prints formatted intersection line to stdout.
  Specifically made for handleIntersectCommand*/
void putIntersectionLine(const Stop* stop,
	DLinkedList* routes);

/*=Search functions=*/
/*findStopWithName - Searches for a Stop with the given name.
  Returns a pointer if found, NULL otherwise*/
DLinkedNode* findStopWithName(const char* name,
	DLinkedList* stops);
/*findRouteWithName - Searches for a Route with the given name.
  Returns a pointer if found, NULL otherwise*/
DLinkedNode* findRouteWithName(const char* name,
	DLinkedList* routes);

/*counteRoutesWithStop - ... (Counts routes that contain the given stop)*/
int countRoutesWithStop(const Stop* stop,
	DLinkedList* routes);
/*countRoutesStops - ... (Counts stops in the given route)*/
int countRouteStops(const Route* route);

/*getTotalCost - Determines the total cost of the given route*/
float getTotalCost(const Route* route);
/*getTotalDuration - Determine the total duration of the give route*/
float getTotalDuration(const Route* route);

/*removeStopAndPath - Removes the given stop from the given route
  and patches the node links.
  Specifically made for handleRemoveStopCommand*/
void removeStopAndPatch(Route* route, Stop* stop);

#endif

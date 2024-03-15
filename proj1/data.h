/*
 * File: data.h
 * Author: Diogo Diniz (ist1106196)
 * Description: Data structures and related functions header
*/

#ifndef DATA_H
#define DATA_H

/*===MACROS===*/
/*Allocation sizes*/
#define STOP_NAME_MAX_LEN 51
#define ROUTE_NAME_MAX_LEN 21

/*===OPTION MACROS===*/
/*Uncomment to print empty lines for empty routes*/
/*#define PRINT_EMPTY_ROUTES*/

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
	/*Dlinked list*/
	Connection* prev, *next; /*8-11;12-15*/
	float cost, duration; /*16-19;20-24*/
};

typedef struct
{
	/*Dlinked list edges*/
	Connection* first, *last; /*0-3;4-7*/
	char* name; /*8-11*/
} Route;


/*===FUNCTION DECLARATIONS===*/
/*Constructors*/
Stop* createStop(const double lat, const double lon, char* name);
Connection* createConnection(Stop* start, Stop* end,
	const float cost, const float duration);
Route* createRoute(char* name);

/*Destructors*/
void destroyStop(Stop* stop);
void destroyConnection(Connection* conn);
void destroyRoute(Route* route);

/*Add connection to route if possible and return success*/
char route_addConnection(Route* route, Connection* conn);

/*Print formatted lines to stdout*/
void putStopLine(const Stop* stop, Route** routes, const int routeC);
void putConnectionLine(const Connection* conn);
void putRouteLine(const Route* route); 
void putRouteLineStops(const Route* route);
void putRouteLineStops_rev(const Route* route);
/*Specifically made for handleIntersectCommand*/
void putIntersectionLine(const Stop* stop,
	Route** routes, const int routeC);

/*Search functions, return pointer if found, null otherwise*/
Stop* findStopWithName(const char* name,
	Stop** stops, const int stopC);
Route* findRouteWithName(const char* name,
	Route** routes, const int routeC);

/*Count functions, return count always*/
int countRoutesWithStop(const Stop* stop,
	Route** routes, const int routeC);
int countRouteStops(const Route* route);

/*Total functions, always return total value for a route*/
float getTotalCost(const Route* route);
float getTotalDuration(const Route* route);

#endif

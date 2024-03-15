/*
 * File: dbg.h
 * Author: Diogo Diniz (ist1106196)
 * Description: Debugging functions header
*/

#ifndef DBG_H
#define DBG_H

/*===OPTION MACROS===*/
/*Uncomment to allow capitals in 'inverso'*/
/*#define ALLOW_INVERT_CAPS*/

/*Dumps all structs in a complete annd readable format*/
void DumpStructures(Stop** stops, const int stopC,
	Connection** conns, const int connC,
	Route** routes, const int routeC);

#endif

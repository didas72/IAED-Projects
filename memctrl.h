/*TMP FILE*/

/*
 * File: memctrl.h
 * Author: Diogo Diniz (ist1106196)
 * Description: Wrapper for malloc/calloc/free
 * Notes: Used mainly for debugging memleaks.
 * I'm a fan of doing things myself and avoiding
 * ext3rnal tools (even if it costs me time and headaches).
 *    ^ illegal word, hence the 3
*/

#ifndef MEMCTRL_H
#define MEMCTRL_H


void initMemCtrl(char shouldDo);
void uninitMemCtrl(char shouldDo);
void listAllMem(char shouldDo);
void listUsedMem(char shouldDo);
char anyNotFreed(char shouldDo);

#ifdef CONTROL_MEMORY_ALLOCS
/*Check for stdlib.h*/
#ifdef NULL
#error "stdlib.h has been included!"
#endif /*NULL*/

#include <stdlib.h>

/*Replacement functions*/
void* myMalloc(size_t size, char* name);
void* myCalloc(size_t nitems, size_t size, char* name);
void myFree(void* ptr, char* name);
void* myRealloc(void* ptr, size_t size, char* name);

/*Replacement macros*/
#define M_INIT() initMemCtrl(1)
#define M_UNINIT() uninitMemCtrl(1)
#define M_LIST() listAllMem(1)
#define M_LIST_N_FREED() listUsedMem(1)
#define M_ANY_N_FREED() anyNotFreed(1)
#define M_MALLOC(size, name) myMalloc(size, name)
#define M_CALLOC(nitems, size, name) myCalloc(nitems, size, name)
#define M_FREE(ptr, name) myFree(ptr, name)
#define M_REALLOC(ptr, size, name) myReaalloc(ptr, size, name)
#define M_EXIT(status) exit(status)

#else /*!CONTROL_MEMORY_ALLOCS*/

#include <stdlib.h>

/*Wrap only macros*/
#define M_INIT() initMemCtrl(0)
#define M_UNINIT() uninitMemCtrl(0)
#define M_LIST() listAllMem(0)
#define M_LIST_N_FREED() listUsedMem(0)
#define M_ANY_N_FREED() anyNotFreed(0)
#define M_MALLOC(size, name) malloc(size)
#define M_CALLOC(nitems, size, name) calloc(nitems, size)
#define M_FREE(ptr, name) free(ptr)
#define M_REALLOC(ptr, size, name) reaalloc(ptr, size)
#define M_EXIT(status) exit(status)

#endif /*CONTROL_MEMORY_ALLOCS*/
#endif

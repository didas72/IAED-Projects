/*
 * File: aux.h
 * Author: Diogo Diniz (ist1106196)
 * Description: Auxiliar functions header
*/

#ifndef AUX_H
#define AUX_H


/*===FUNCTION DECLARATIONS===*/
char* copyName(char* src, int maxName, char** afterEndP1);
char* copyNameNoQuotes(char* src, int maxName, char** afterEndP1);
double copyFloat(char* src, char** afterEndP1);

/*Specifically made for handleRouteCommand*/
char isInvert(char* rem);

/*Kept for reference (unnecessary given git)*/
char* copyName_old(char* src, char** afterEndP1);

#endif

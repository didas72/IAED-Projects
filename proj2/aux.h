/*
 * File: aux.h
 * Author: Diogo Diniz (ist1106196)
 * Description: Auxiliar functions header
*/

#ifndef AUX_H
#define AUX_H


/*===FUNCTION DECLARATIONS===*/
/*copyName - Returns a pointer to a new string containing a copy
  of the first sequence between brackets or spaces.*/
char* copyName(char* src, char** afterEndP1);
/*copyNameNoQuotes - Returns a pointer to a new string containing a copy
  of the first sequence between spaces.*/
char* copyNameNoQuotes(char* src, char** afterEndP1);
/*copyFloat - Returns the first double value present in the input string*/
double copyFloat(char* src, char** afterEndP1);

/*isInvert - Checks if the given string is compatible with inverso.
  Specifically made for handleRouteCommand*/
char isInvert(char* rem);
#endif

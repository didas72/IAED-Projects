/*
 * File: aux.c
 * Author: Diogo Diniz (ist1106196)
 * Description: Implementation of functions in aux.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "aux.h"


/*===FUNCTION IMPLEMENTATIONS===*/
char* copyName(char* src, int maxName, char** afterEndP1)
{
	/*Copy to new location, no need to free, ptr will be kept in the structs*/
	char* tmp = malloc(maxName * sizeof(char));

	if (*src == '"')
	{
		sscanf(src, "\"%[^\"]\"", tmp);
		*afterEndP1 = src + strlen(tmp) + 3;
	}
	else
	{
		sscanf(src, "%s", tmp);
		*afterEndP1 = src + strlen(tmp) + 1;
	}
	return tmp;
}

char* copyNameNoQuotes(char* src, int maxName, char** afterEndP1)
{
	/*Copy to new location, no need to free, ptr will be kept in the structs*/
	char* tmp = malloc(maxName * sizeof(char));

	sscanf(src, "%s", tmp);
	*afterEndP1 = src + strlen(tmp) + 1;
	
	return tmp;
}

double copyFloat(char* src, char** afterEndP1)
{
	/*TODO: (Maybe) Replace with sscanf and REGEX*/

	char* end, *tmp;
	double ret;

	/*Skip whitespaces*/
	while (*src == ' ' || *src == '\t')
		src++;

	if (!(end = strchr(src, ' ')))
	{
		/*Parse entire string*/
		*afterEndP1 = &src[strlen(src)];
		return atof(src);
	}
	else
	{
		/*Parse only section*/
		/*Length in bytes is pointer distance - ending space*/
		if (!(tmp = malloc(end - src))) return -1.0f;

		/*Swap last quote for \0 to end string*/
		*end = 0;

		/*Copy str over, except first quote*/
		strcpy(tmp, src);

		/*Restore quote*/
		*end = '"';

		*afterEndP1 = ++end;

		/*Parse and free tmp*/
		ret = atof(tmp);
		free(tmp);

		return ret;
	}
}

char isInvert(char* rem)
{
	char* inv = "inverso";
	int i = -1, len = strlen(rem);

	if (len < 3 || len > 7) /*Length of 'inverso', not worth a macro*/
		return 0;

	while (inv[++i])
	{
		if (!(rem[i]))
			break;

		#ifdef ALLOW_INVERT_CAPS
		if (tolower(rem[i]) != inv[i])
			return 0;
		#else
		if (rem[i] != inv[i])
			return 0;
		#endif
	}

	return 1;
}

char* copyName_old(char* src, char** afterEndP1)
{
	char* end, *ret, retSpc = 1;

	if (*src == '"')
	{
		/*If starts with quotes*/
		end = strchr(src+1, '"');
		ret = malloc(end-src-1); /*Length + 2 (quotes) - 1*/

		/*Copy until quote*/
		*end = 0;
		strcpy(ret, src+1);
		*end = '"';

		/*Return and set args*/
		*afterEndP1 = end+2;
		return ret;
	}
	else
	{
		/*If no quotes*/
		end = strchr(src, ' ');
		/*Handle null case (have to copy untill the end)*/
		if (!end)
		{
			retSpc = 0;
			end = src + strlen(src) + 1;
		}

		ret = malloc(end - src); /*Length + 1 (end space)*/

		/*Copy until space*/
		*end = 0;
		strcpy(ret, src);
		if (retSpc)
			*end = ' ';

		/*Return and set args*/
		*afterEndP1 = end+1;
		return ret;
	}
}

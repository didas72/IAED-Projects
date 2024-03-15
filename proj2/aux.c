/*
 * File: aux.c
 * Author: Diogo Diniz (ist1106196)
 * Description: Implementation of functions in aux.h
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "aux.h"
#include "errors.h"


/*===FUNCTION IMPLEMENTATIONS===*/
/*copyName - Dynamically determine str length with pointer distance,
  allocate needed space and copy string. Looks both for quotes and spaces*/
char* copyName(char* src, char** afterEndP1)
{
	char* end, *ret, retSpc = 1;

	if (*src == '"')
	{
		/*If starts with quotes*/
		end = strchr(src+1, '"');
		/*Length - 1 (quotes) + 1*/
		ret = malloc(end-src);

		CHECK_ALLOC(ret);

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
		char* space, *tab;
		space = strchr(src+1, ' ');
		tab = strchr(src+1, '\t');
		if (space == 0)
			end = tab;
		else if (tab == 0)
			end = space;
		else
			end = space < tab ? space : tab;

		/*Handle null case (have to copy until the end)*/
		if (!end)
		{
			retSpc = 0;
			end = src + strlen(src);
		}

		/*Length + 1*/
		ret = malloc(end-src+1);
	
		CHECK_ALLOC(ret);

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
/*copyNameNoQuotes - Dynamically determine str length with pointer distance,
  allocate needed space and copy string*/
char* copyNameNoQuotes(char* src, char** afterEndP1)
{
	char* end, *ret, retSpc = 1;

	/*If no quotes*/
	char* space, *tab;
	space = strchr(src+1, ' ');
	tab = strchr(src+1, '\t');
	if (space == 0)
		end = tab;
	else if (tab == 0)
		end = space;
	else
		end = space < tab ? space : tab;

	/*Handle null case (have to copy until the end)*/
	if (!end)
	{
		retSpc = 0;
		end = src + strlen(src) + 1;
	}

	/*Length + 1*/
	ret = malloc(end - src + 1);

	CHECK_ALLOC(ret);

	/*Copy until space*/
	*end = 0;
	strcpy(ret, src);
	if (retSpc)
		*end = ' ';

	/*Return and set args*/
	*afterEndP1 = end+1;
	return ret;
}
/*copyFloat - Parse double from string with sscanf*/
double copyFloat(char* src, char** afterEndP1)
{
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
		/*Length in bytes is pointer distance + 0*/
		tmp = malloc(end - src + 1);

		CHECK_ALLOC(tmp);

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
/*isInvert - Iteratively check each character until end of strings*/
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

		if (rem[i] != inv[i])
			return 0;
	}

	return 1;
}

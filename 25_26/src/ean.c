#include "ean.h"

#include <string.h>
#include <ctype.h>

#define EAN13_CHARS 13
#define EAN8_CHARS 8

ean_t ean_parse(char *str)
{
	size_t len = strlen(str);
	ean_t ean = 0;
	int check = 0;

	if (len != EAN13_CHARS && len != EAN8_CHARS)
		return EAN_NVAL;

	for (size_t i = 0; i < len; ++i)
	{
		if (!isdigit(str[i]))
			return EAN_NVAL;

		ean = (ean << 4) | (str[i] - '0');
		if (i != len - 1)
			check += (str[i] - '0') * ((i % 2 == 1) ? 3 : 1);
	}

	check = (10 - (check % 10)) % 10;

	if (check != (int)(ean & 0xF))
		return EAN_NVAL;

	return ean;
}

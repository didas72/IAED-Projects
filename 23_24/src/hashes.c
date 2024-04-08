#include "hashes.h"

#include <stddef.h>
#include <string.h>

#include "structs.h"

size_t hash_str(void *ptr)
{
	char *str = ptr;
	size_t hash = *str;

	while (*(++str)) hash = hash * 41 + *str;

	return hash;
}
int compare_str(void *ptr1, void *ptr2)
{
	return strcmp((char *)ptr1, (char *)ptr2);
}

size_t hash_timestamp(void *ptr)
{
	return (size_t)ptr;
}
int compare_timestamp(void *ptr1, void *ptr2)
{
	return timestamp_compare((timespan_t)ptr1, (timespan_t)ptr2);
}

#include "aux.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#ifndef USE_ALLOC_CHECK
#define USE_STANDARD_MEM
#endif
#include "alloc_check/alloc_check.h"

#include "structs.h"



#define MAX_ARGS 16



static char *argv[MAX_ARGS];
static char buff[BUFSIZ+1];

int get_args(char ***s_argv, int *s_argc)
{
	int buffh = 0, argc = 0, read;
	char ch;

	memset(argv, 0, MAX_ARGS * sizeof(char *));

	while((read = getchar()) != '\n' && read != EOF)
		buff[buffh++] = (char)read;

	if (buffh == 0)
		return 1;

	buff[buffh] = '\0';
	buffh = 0;
	argv[0] = NULL;

	while ((ch = buff[buffh]) != '\0')
	{
		if (ch == '"')
		{
			buff[buffh] = '\0';
			argv[argc++] = &buff[++buffh];
			while (buff[buffh] != '\0' && buff[buffh++] != '"');
			buff[buffh-1] = '\0';
		}
		else if (ch == ' ' || ch == '\t')
		{
			buff[buffh] = '\0';
			buffh++;
			if (argv[argc] != NULL)
				argc++;
		}
		else if (argv[argc] == NULL)
		{
			argv[argc] = &buff[buffh];
			buffh++;
		}
		else
			buffh++;
	}

	if (argv[argc] != NULL && *argv[argc] != '\0')
		argc++;

	*s_argv = argv;
	*s_argc = argc;

	return 0;
}

static park_t *_movement_park_match_park = NULL;
void set_movent_park_match_park(park_t *ptr)
{
	_movement_park_match_park = ptr;
}
int movement_park_match(void *ptr)
{
	return ((movement_t *)ptr)->park == _movement_park_match_park;
}

int park_id_comparer(void *ptr1, void *ptr2)
{
	size_t first = ((park_t*)ptr1)->id, second = ((park_t*)ptr2)->id;
	return first < second ? -1 : (first > second ? 1 : 0);
}

char validate_license_plate(char *plate)
{
	if (strlen(plate) != 8)
		return 1;

	if (plate[2] != '-' || plate[5] != '-')
		return 1;

	char n = 0, l = 0;
	for (int i = 0; i < 3; i++)
	{
		if (isdigit(plate[i * 3]) && isdigit(plate[i * 3 + 1]))
			n++;
		else if (isalpha(plate[i * 3]) && isalpha(plate[i * 3 + 1]))
			l++;
		else
			return 1;
	}
	return !((n == 2 && l == 1) || (n == 1 && l == 2));
}

char *my_strdup(char *str)
{
	char *ret = CHKD_MALLOC(strlen(str) + 1);
	strcpy(ret, str);
	return ret;
}

double determine_cost(movement_t *movement)
{
	park_t *park = movement->park;
	timespan_t parked_time = timestamp_get_elapsed(movement->entry, movement->exit);
	size_t periods = timespan_get_periods(parked_time);

	double periods_cost = MIN(periods, 4) * park->cost_f15 +
		(periods > 4 ? periods - 4 : 0) * park->cost_n15;
	double cost = 
		timespan_get_days(parked_time) * park->cost_max +
		MIN(periods_cost, park->cost_max);

	return cost;
}

int movement_compare(void *ptr1, void *ptr2)
{
	movement_t *first = ptr1, *second = ptr2;

	int cmp = strcmp(first->park->name, second->park->name);

	return cmp ? cmp : (timestamp_compare(first->entry, second->entry));
}
int billing_day_compare(void *ptr1, void *ptr2)
{
	billing_day_t *first = ptr1, *second = ptr2;

	return timestamp_compare(first->date, second->date);
}
int park_compare(void *ptr1, void *ptr2)
{
	park_t *first = ptr1, *second = ptr2;

	return strcmp(first->name, second->name);
}

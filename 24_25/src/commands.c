#include "commands.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sus/hashtable.h>
#include <sus/vector.h>

#include "structs.h"

void cmd_create(state_t *state, char **argv, size_t argc)
{
#ifdef DEBUG
	if (argc != 4)
	{
		fprintf(stderr, "Command 'c' requires exactly 4 arguments");
		abort();
	}
#endif

	size_t vaccince_count = hashtable_get_count(state->vaccines);
	if (vaccince_count >= MAX_VACCINES)
	{
		ERR(ERR_MAX_VACCINES);
		return;
	}

	batch_t batch;
	date_t date;
	int count;
	char *name = argv[3];

	if (parse_batch(argv[0], &batch))
	{
		ERR(ERR_BATCH_NVAL);
		return;
	}

	if (hashtable_has_key(state->vaccines, &batch))
	{
		ERR(ERR_BATCH_DUP);
		return;
	}

	if (invalid_vaccine_name(name))
	{
		ERR(ERR_NAME_NVAL);
		return;
	}

	if ((date = parse_date(argv[1])) == DATE_NVAL)
	{
		ERR(ERR_DATE_NVAL);
		return;
	}

	if (sscanf(argv[2], "%d", &count) != 1 || count <= 0)
	{
		ERR(ERR_COUNT_NVAL);
		return;
	}

	vaccine_t *vaccine = vaccine_create(batch, date, count, name);
	hashtable_add(state->vaccines, &vaccine->batch, vaccine); //REVIEW: Maybe check return code (in case of failed alloc)
}

static void list_all(state_t *state);
static void list_selected(state_t *state, char **argv, size_t argc);
void cmd_list(state_t *state, char **argv, size_t argc)
{
	if (argc == 0)
		list_all(state);
	else
		list_selected(state, argv, argc);
}



static int vaccine_comparer(void *first, void *second)
{
	vaccine_t *a = first, *b = second;

	if (a->expiration_date != b->expiration_date)
		return a->expiration_date - b->expiration_date;
	
	return batch_comprarer(a, b);
}

static int vaccine_filter(void *vac, void *arg)
{
	vaccine_t *vaccine = vac;
	char *name = arg;

	return !strcmp(vaccine->name, name);
}

static void list_all(state_t *state)
{
	//vector_t<vaccine_t *>
	vector_t *vaccines = hashtable_list_contents(state->vaccines);

	vector_sort(vaccines, vaccine_comparer); //REVIEW: Might be broken

	for (size_t i = 0; i < vaccines->count; ++i)
	{
		vaccine_t *vaccine = vaccines->data[i];
		print_vaccine(vaccine, 0); //TODO: Properly populate times applied
		putchar('\n');
	}

	vector_destroy(vaccines);
}

static void list_selected(state_t *state, char **argv, size_t argc)
{
	//vector_t<vaccine_t *>
	vector_t *vaccines = hashtable_list_contents(state->vaccines);

	for (size_t i = 0; i < argc; ++i)
	{
		//vector_t<vaccine_t *>
		vector_t *matches = vector_get_all(vaccines, vaccine_filter, argv[i]);
		vector_sort(matches, vaccine_comparer); //REVIEW: Might be broken
		for (size_t j = 0; j < matches->count; ++j)
		{
			vaccine_t *vaccine = matches->data[j];
			print_vaccine(vaccine, 0); //TODO: Properly populate times applied
			putchar('\n');
		}
		vector_destroy(matches);
	}

	vector_destroy(vaccines);
}

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
#else
	(void)argc;
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

	if (date < state->current_date)
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
	state_add_vaccine(state, vaccine);

	print_batch(vaccine->batch); putchar('\n');
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

void cmd_apply(state_t *state, char **argv, size_t argc)
{
#ifdef DEBUG
	if (argc != 2)
	{
		fprintf(stderr, "Command 'a' requires exactly 2 arguments");
		abort();
	}
#else
	(void)argc;
#endif

	char *user_name = argv[0];
	char *vaccine_name = argv[1];

	vaccine_t *vaccine = state_get_vaccine(state, vaccine_name);
	if (!vaccine)
	{
		ERR(ERR_DEPLETED);
		return;
	}

	if (vaccine->available <= 0)
	{
		ERR(ERR_DEPLETED);
		return;
	}
	
	//TODO: abstract
	//vector_t<inoculation_t*>
	vector_t *inoculations = hashtable_get(state->user_to_inoc, user_name);
	if (inoculations != NULL && inoculations->count != 0)
	{
		for (size_t i = inoculations->count - 1; i != ~0ul; --i)
		{
			inoculation_t *inoc = inoculations->data[i];
			if (inoc->date != state->current_date)
				break;

			vaccine_t *other_vaccine = hashtable_get(state->vaccines, &inoc->batch);
			if (strcmp(vaccine_name, other_vaccine->name))
				continue;

			ERR(ERR_VACCINATED);
			return;
		}
	}

	inoculation_t *inoculation = inoculation_create(vaccine->batch, state->current_date, user_name);
	state_add_inoculation(state, inoculation);

	--vaccine->available;
	++vaccine->applied;
	print_batch(vaccine->batch);
	putchar('\n');
}

void cmd_time(state_t *state, char **argv, size_t argc)
{
#ifdef DEBUG
	if (argc != 1 && argc != 0)
	{
		fprintf(stderr, "Command 't' requires 0 or 1 arguments");
		abort();
	}
#endif

	if (argc == 1)
	{
		date_t date;

		if ((date = parse_date(argv[0])) == DATE_NVAL)
		{
			ERR(ERR_DATE_NVAL);
			return;
		}

		if (date < state->current_date)
		{
			ERR(ERR_DATE_NVAL);
			return;
		}

		state->current_date = date;
	}

	print_date(state->current_date);
	putchar('\n');
}

static void user_all(state_t *state);
static void user_selected(state_t *state, char *username);
void cmd_user(state_t *state, char **argv, size_t argc)
{
#ifdef DEBUG
	if (argc != 1 && argc != 0)
	{
		fprintf(stderr, "Command 'u' requires 0 or 1 arguments");
		abort();
	}
#endif

	if (argc == 0)
		user_all(state);
	else
		user_selected(state, argv[0]);
}

void cmd_remove(state_t *state, char **argv, size_t argc)
{
#ifdef DEBUG
	if (argc != 1)
	{
		fprintf(stderr, "Command 'r' requires exactly 1 arguments");
		abort();
	}
#else
	(void)argc;
#endif

	batch_t batch;

	if (parse_batch(argv[0], &batch))
	{
		ERR_ARG(ERR_NO_BATCH, argv[0]);
		return;
	}

	vaccine_t *vaccine = hashtable_get(state->vaccines, &batch);
	if (vaccine == NULL)
	{
		ERR_ARG(ERR_NO_BATCH, argv[0]);
		return;
	}

	printf("%d\n", vaccine->applied);

	if (vaccine->applied == 0)
	{
		state_remove_vaccine(state, vaccine);
		vaccine_destroy(vaccine);
	}
	else
	{
		vaccine->available = 0;
	}
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
		print_vaccine(vaccine);
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

		if (matches->count == 0)
		{
			ERR_ARG(ERR_NO_VACCINE, argv[i]);
		}

		vector_sort(matches, vaccine_comparer); //REVIEW: Might be broken
		for (size_t j = 0; j < matches->count; ++j)
		{
			vaccine_t *vaccine = matches->data[j];
			print_vaccine(vaccine);
			putchar('\n');
		}
		vector_destroy(matches);
	}

	vector_destroy(vaccines);
}

static void user_all(state_t *state)
{
	for (size_t i = 0; i < state->inoculations->count; ++i)
	{
		inoculation_t *inoc = state->inoculations->data[i];
		print_inoculation(inoc); putchar('\n');
	}
}

static void user_selected(state_t *state, char *username)
{
	//TODO: abstract
	//vector_t<inoculation_t*>
	vector_t *inoculations = hashtable_get(state->user_to_inoc, username);

	if (inoculations == NULL)
	{
		ERR_ARG(ERR_NO_USER, username);
		return;
	}

	for (size_t i = 0; i < inoculations->count; ++i)
	{
		inoculation_t *inoc = inoculations->data[i];
		print_inoculation(inoc); putchar('\n');
	}
}

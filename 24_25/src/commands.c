#include "commands.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sus/hashtable.h>

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

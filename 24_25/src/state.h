#ifndef STATE_H_
#define STATE_H_

#include <stdlib.h>

#include <sus/hashtable.h>
#include <sus/vector.h>
#include <sus/hashes.h>

#include "structs.h"

#define MAX_VACCINES 1000

typedef struct state_t
{
	//hashtable_t<batch_t*, vaccine_t*> - owns values, keys owned by values
	hashtable_t *vaccines;
	//vector_t<inoculation_t*> - owns values
	vector_t *inoculations;
	//hashtable_t<batch_t*, vector_t*<inoculation_t*>> - owns values, keys owned by values of vaccines
	hashtable_t *batch_to_inoc;
	//hashtable_t<char*, vector_t*<inoculation_t*>> - owns values, owns keys
	hashtable_t *user_to_inoc;
	date_t current_date;
	//TODO: Error translation table
} state_t;

state_t *state_init()
{
	state_t *state = malloc(sizeof(state_t));

	if (!state)
		return NULL;

	state->current_date = 0; //Trust
	state->vaccines = hashtable_create(, ); //TODO: Hasher and comprarer for batch_t
	state->inoculations = vector_create();
	state->batch_to_inoc = hashtable_create(, ); //TODO: Hasher and comparer for batch_t
	state->user_to_inoc = hashtable_create(hash_str, compare_str);

	return state;
}

void state_destroy(state_t *state)
{
	hashtable_destroy_free(state->vaccines, NULL, free);
	vector_destroy_free(state->inoculations, ); //TODO: Freer for inoculation_t
	hashtable_destroy_free(state->batch_to_inoc, NULL, vector_destroy);
	hashtable_destroy_free(state->user_to_inoc, free, vector_destroy);

	free(state);
}

#endif

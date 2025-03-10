#include "state.h"

state_t *state_init()
{
	state_t *state = malloc(sizeof(state_t));

	if (!state)
		return NULL;

	state->current_date = 0; //Trust
	state->vaccines = hashtable_create(batch_hasher, batch_comprarer);
	state->inoculations = vector_create();
	state->batch_to_inoc = hashtable_create(batch_hasher, batch_comprarer);
	state->user_to_inoc = hashtable_create(hash_str, compare_str);

	return state;
}

void state_destroy(state_t *state)
{
	hashtable_destroy_free(state->vaccines, NULL, free);
	vector_destroy_free(state->inoculations, (void (*) (void*))inoculation_destroy); //TODO: Freer for inoculation_t
	hashtable_destroy_free(state->batch_to_inoc, NULL, (void (*) (void*))vector_destroy); //REVIEW: Unexpected in return might overwrite EAX
	hashtable_destroy_free(state->user_to_inoc, free, (void (*) (void*))vector_destroy); //REVIEW: Unexpected in return might overwrite EAX

	free(state);
}

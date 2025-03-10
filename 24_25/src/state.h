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

state_t *state_init();

void state_destroy(state_t *state);

#endif

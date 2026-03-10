#ifndef STATE_H_
#define STATE_H_

#include <stdlib.h>

#include <sus/hashtable.h>
#include <sus/vector.h>

#include "structs.h"
#include "error_utils.h"

#define MAX_VACCINES 1000

#define LOCALE_EN 0
#define LOCALE_PT 1

typedef struct state_t
{
	//hashtable_t<batch_t*, vaccine_t*> - owns values, keys owned by values
	hashtable_t *vaccines;
	//hashtable_t<char*, vector_t<vaccine_t*>> - owns values and keys
	hashtable_t *name_to_vaccine;
	//vector_t<inoculation_t*> - owns values
	vector_t *inoculations;
	//hashtable_t<char*, vector_t*<inoculation_t*>> - owns values, owns keys
	hashtable_t *user_to_inoc;
	date_t current_date;
	char *error_locales[ERR_MAX];
} state_t;

state_t *state_create(int locale);
void state_destroy(state_t *state);

void state_add_vaccine(state_t *state, vaccine_t *vaccine);
vaccine_t *state_get_vaccine(state_t *state, char *name);
void state_remove_vaccine(state_t *state, vaccine_t *vaccine);

void state_add_inoculation(state_t *state, inoculation_t *inoc);
size_t state_remove_inoculations(state_t *state, char *username, date_t date, batch_t batch);

#endif

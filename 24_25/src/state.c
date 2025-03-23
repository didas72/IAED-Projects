#include "state.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sus/vector.h>
#include <sus/hashtable.h>
#include <sus/hashes.h>
#include <sus/sus.h>

#include "structs.h"
#include "error_utils.h"

state_t *state_create(int locale)
{
	state_t *state = malloc(sizeof(state_t));

	if (!state)
		return NULL;

	state->current_date = 0; //Trust
	state->vaccines = hashtable_create(batch_hasher, batch_comprarer);
	state->name_to_vaccine = hashtable_create(hash_str, compare_str);
	state->inoculations = vector_create();
	state->user_to_inoc = hashtable_create(hash_str, compare_str);

	switch (locale)
	{
		case LOCALE_EN:
			state->error_locales[ERR_NO_MEMORY] = "no memory";
			state->error_locales[ERR_MAX_VACCINES] = "too many vaccines";
			state->error_locales[ERR_BATCH_DUP] = "duplicate batch number";
			state->error_locales[ERR_BATCH_NVAL] = "invalid batch";
			state->error_locales[ERR_NAME_NVAL] = "invalid name";
			state->error_locales[ERR_DATE_NVAL] = "invalid date";
			state->error_locales[ERR_COUNT_NVAL] = "invalid quantity";
			state->error_locales[ERR_NO_VACCINE] = "%s: no such vaccine";
			state->error_locales[ERR_DEPLETED] = "no stock";
			state->error_locales[ERR_VACCINATED] = "already vaccinated";
			state->error_locales[ERR_NO_BATCH] = "%s: no such batch";
			state->error_locales[ERR_NO_USER] = "%s: no such user";
			break;

		case LOCALE_PT:
			state->error_locales[ERR_NO_MEMORY] = "sem memória";
			state->error_locales[ERR_MAX_VACCINES] = "demasiadas vacinas";
			state->error_locales[ERR_BATCH_DUP] = "número de lote duplicado";
			state->error_locales[ERR_BATCH_NVAL] = "lote inválido";
			state->error_locales[ERR_NAME_NVAL] = "nome inválido";
			state->error_locales[ERR_DATE_NVAL] = "data inválida";
			state->error_locales[ERR_COUNT_NVAL] = "quantidade inválida";
			state->error_locales[ERR_NO_VACCINE] = "%s: vacina inexistente";
			state->error_locales[ERR_DEPLETED] = "esgotado";
			state->error_locales[ERR_VACCINATED] = "já vacinado";
			state->error_locales[ERR_NO_BATCH] = "%s: lote inexistente";
			state->error_locales[ERR_NO_USER] = "%s: utente inexistente";
			break;

		default:
#ifdef DEBUG
			fprintf(stderr, "Invalid locale proivded\n");
#endif
			abort();
	}

	return state;
}
void state_destroy(state_t *state)
{
	hashtable_destroy_free(state->vaccines, NULL, free);
	hashtable_destroy_free(state->name_to_vaccine, free, (void (*) (void*))vector_destroy);
	vector_destroy_free(state->inoculations, (void (*) (void*))inoculation_destroy);
	hashtable_destroy_free(state->user_to_inoc, free, (void (*) (void*))vector_destroy);

	free(state);
}

void state_add_vaccine(state_t *state, vaccine_t *vaccine)
{
	hashtable_add(state->vaccines, &vaccine->batch, vaccine);

	//vector_t<vaccine_t*>
	vector_t *vaccines_with_name = hashtable_get(state->name_to_vaccine, vaccine->name);
	if (vaccines_with_name == NULL)
	{
		vaccines_with_name = vector_create();
		char *name = strdup(vaccine->name);
		hashtable_add(state->name_to_vaccine, name, vaccines_with_name);
	}
	vector_append(vaccines_with_name, vaccine);
}
static int available_vaccine_filter(void *vac, void *arg);
static int oldest_vaccine_comparer(void *first, void *second);
vaccine_t *state_get_vaccine(state_t *state, char *name)
{
	//vector_t<vaccine_t*>
	vector_t *vaccines = hashtable_get(state->name_to_vaccine, name);

	if (!vaccines)
	{
		return NULL;
	}

	//vector_t<vaccine_t*>
	vector_t *available_vaccines = vector_get_all(vaccines, available_vaccine_filter, state);

	if (available_vaccines->count == 0)
	{
		vector_destroy(available_vaccines);
		return NULL;
	}

	vector_sort(available_vaccines, oldest_vaccine_comparer);
	vaccine_t *vaccine = available_vaccines->data[0];

	vector_destroy(available_vaccines);
	return vaccine;
}
void state_remove_vaccine(state_t *state, vaccine_t *vaccine)
{
	hashtable_remove(state->vaccines, &vaccine->batch, NULL, NULL);

	//vector_t<vaccine_t*>
	vector_t *vaccines = hashtable_get(state->name_to_vaccine, vaccine->name);
	vector_remove(vaccines, vaccine);
}

void state_add_inoculation(state_t *state, inoculation_t *inoc)
{
	vector_append(state->inoculations, inoc);

	//vector_t<vaccine_t*>
	vector_t *inoculations_with_user = hashtable_get(state->user_to_inoc, inoc->name);
	if (inoculations_with_user == NULL)
	{
		inoculations_with_user = vector_create();
		char *name = strdup(inoc->name);
		hashtable_add(state->user_to_inoc, name, inoculations_with_user);
	}
	vector_append(inoculations_with_user, inoc);
}
static size_t binary_search_date_start(vector_t *inocs, date_t date);
size_t state_remove_inoculations(state_t *state, char *username, date_t date, batch_t batch)
{ //REVIEW: Should empty users be deleted? Assumes yes
	//vectpr_t<inoculation_t*>
	vector_t *selected = vector_create();
	//vector_t<inoculation_t*>
	vector_t *inocs = hashtable_get(state->user_to_inoc, username);
	inoculation_t *inoc;

	//Each condition handles cleanup of inocs (as it is optimizable)
	//Conditions can expect trailing code to remove user if empty
	if (date == DATE_NVAL) //Only username was passed
	{
		vector_append_vector(selected, inocs);
		vector_clear(inocs);
	}
	else if (batch_invalid(batch)) //Username and date passed
	{
		size_t start = binary_search_date_start(inocs, date);
		if (start == ~0ul)
		{
			vector_destroy(selected);
			return 0;
		}
		size_t count = 1;

		//OPTIMIZE: Replace with binary search as well, but with low = start
		//NOTE: Max iterations is 1000 (max different vaccines, all inocs in a day must be unique)
		for (size_t i = start + 1; i < inocs->count; ++i, ++count)
		{
			inoc = inocs->data[i];
			if (inoc->date != date)
				break;
		}

		vector_append_range(selected, inocs, start, count);
		vector_remove_range(inocs, start, count);
	}
	else //Username, date and batch passed
	{
		size_t start = binary_search_date_start(inocs, date);
		if (start == ~0ul)
		{
			vector_destroy(selected);
			return 0;
		}

		for (size_t i = start; i < inocs->count; ++i)
		{
			inoc = inocs->data[i];

			if (batch_comprarer(&inoc->batch, &batch))
				continue;

			vector_append(selected, inoc);
			vector_remove(inocs, inoc);
			break;
		}
	}

	//NOTE: Given that inocs, and conseguently selected, have the same order as state->inoculations, 
	//NOTE:   and that vector_remove shifts higher indices down to lower ones if a gap is formed,
	//NOTE:   removing inoculations in reverse order yields the best performance
	for (size_t i = selected->count - 1; i != ~0ul; --i)
		vector_remove(state->inoculations, selected->data[i]);
	
	//Remove user if empty
	if (inocs->count == 0)
	{
		char *key;
		hashtable_remove(state->user_to_inoc, username, (void**)&key, NULL);
		vector_destroy(inocs);
		free(key);
	}

	size_t removed_count = selected->count;
	vector_destroy_free(selected, (void (*)(void*))inoculation_destroy);
	return removed_count;
}


// Auxiliar functions
static int available_vaccine_filter(void *vac, void *arg)
{
	vaccine_t *vaccine = vac;
	state_t *state = arg;

	return vaccine->available > 0 && vaccine->expiration_date > state->current_date;
}
static int oldest_vaccine_comparer(void *first, void *second)
{
	vaccine_t *a = first, *b = second;

	return a->expiration_date - b->expiration_date;
}

static size_t binary_search_date_start(vector_t *inocs, date_t date)
{ //NOTE: Assumes inocs won't be empty
	size_t i = 0;
	size_t high = inocs->count - 1, low = 0;
	inoculation_t *inoc = inocs->data[0];

	while (high - low)
	{
		i = (high + low) >> 1;
		inoc = inocs->data[i];

		if (inoc->date < date)
			low = i + 1;
		else //If high or match
			high = i;
	}

	inoc = inocs->data[low];

	if (inoc->date != date) //Requested date not found
		return ~(size_t)0;

	return low;
}

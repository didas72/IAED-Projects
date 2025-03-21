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
	state->batch_to_inoc = hashtable_create(batch_hasher, batch_comprarer);
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
	hashtable_destroy_free(state->name_to_vaccine, free, (void (*) (void*))vector_destroy); //REVIEW: Unexpected in return might overwrite EAX
	vector_destroy_free(state->inoculations, (void (*) (void*))inoculation_destroy);
	hashtable_destroy_free(state->batch_to_inoc, NULL, (void (*) (void*))vector_destroy); //REVIEW: Unexpected in return might overwrite EAX
	hashtable_destroy_free(state->user_to_inoc, free, (void (*) (void*))vector_destroy); //REVIEW: Unexpected in return might overwrite EAX

	free(state);
}

void state_add_vaccine(state_t *state, vaccine_t *vaccine)
{
	hashtable_add(state->vaccines, &vaccine->batch, vaccine); //REVIEW: Maybe check return code (in case of failed alloc)

	//vector_t<vaccine_t*>
	vector_t *vaccines_with_name = hashtable_get(state->name_to_vaccine, vaccine->name);
	if (vaccines_with_name == NULL)
	{
		vaccines_with_name = vector_create(); //REVIEW: Maybe check return code (in case of failed alloc)
		char *name = strdup(vaccine->name);
		hashtable_add(state->name_to_vaccine, name, vaccines_with_name); //REVIEW: Maybe check return code (in case of failed alloc)
	}
	vector_append(vaccines_with_name, vaccine); //REVIEW: Maybe check return code (in case of failed alloc)
}
void state_add_inoculation(state_t *state, inoculation_t *inoc)
{
	vector_append(state->inoculations, inoc); //REVIEW: Maybe check return code (in case of failed alloc)

	//vector_t<inoculation_t*>
	vector_t *inoculations_with_batch = hashtable_get(state->batch_to_inoc, &inoc->batch);
	if (inoculations_with_batch == NULL)
	{
		inoculations_with_batch = vector_create();
		batch_t *batch = &inoc->batch; //TODO: Unscrew key ownership (should not be owned by inoculation in case it gets removed)
		hashtable_add(state->batch_to_inoc, batch, inoculations_with_batch); //REVIEW: Maybe check return code (in case of failed alloc)
	}
	vector_append(inoculations_with_batch, inoc); //REVIEW: Maybe check return code (in case of failed alloc)

	//vector_t<vaccine_t*>
	vector_t *inoculations_with_user = hashtable_get(state->user_to_inoc, inoc->name);
	if (inoculations_with_user == NULL)
	{
		inoculations_with_user = vector_create();
		char *name = strdup(inoc->name);
		hashtable_add(state->user_to_inoc, name, inoculations_with_user); //REVIEW: Maybe check return code (in case of failed alloc)
	}
	vector_append(inoculations_with_user, inoc); //REVIEW: Maybe check return code (in case of failed alloc)
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

	vector_sort(available_vaccines, oldest_vaccine_comparer); //REVIEW: Check sort order
	vaccine_t *vaccine = available_vaccines->data[0];

	vector_destroy(available_vaccines);
	return vaccine;
}

static int available_vaccine_filter(void *vac, void *arg)
{
	vaccine_t *vaccine = vac;
	state_t *state = arg;

	return vaccine->count > 0 && vaccine->expiration_date > state->current_date;
}
static int oldest_vaccine_comparer(void *first, void *second)
{
	vaccine_t *a = first, *b = second;

	return a->expiration_date - b->expiration_date;
}

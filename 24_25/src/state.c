#include "state.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error_utils.h"

state_t *state_create(int locale)
{
	state_t *state = malloc(sizeof(state_t));

	if (!state)
		return NULL;

	state->current_date = 0; //Trust
	state->vaccines = hashtable_create(batch_hasher, batch_comprarer);
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
	vector_destroy_free(state->inoculations, (void (*) (void*))inoculation_destroy);
	hashtable_destroy_free(state->batch_to_inoc, NULL, (void (*) (void*))vector_destroy); //REVIEW: Unexpected in return might overwrite EAX
	hashtable_destroy_free(state->user_to_inoc, free, (void (*) (void*))vector_destroy); //REVIEW: Unexpected in return might overwrite EAX

	free(state);
}

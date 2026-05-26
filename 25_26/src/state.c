#include "state.h"

#include <stdio.h>
#include <string.h>

#include <sus/hashtable.h>
#include <sus/hashes.h>
#include <sus/ivector.h>

int state_init(state_t *state)
{
	memset(state, 0, sizeof(*state));

	state->iva_map = hashtable_create(hash_ptr, compare_ptr);
	state->product_map = hashtable_create(hash_ptr, compare_ptr);
	state->product_vec = ivector_create(sizeof(product_t*));
	state->invoices = ivector_create(sizeof(invoice_t*));

	return 0;
}

int state_load_iva(state_t *state, char *path)
{
	FILE *file = fopen(path, "r");

	if (file == NULL)
		return -1;

	char line[512];

	while (fgets(line, sizeof(line), file) != NULL)
	{
		char letter = 0;
		iva_t percent = 0;
		sscanf(line, "%c %hhd", &letter, &percent);

		hashtable_add(state->iva_map, (void*)(uint64_t)letter, (void*)(uint64_t)percent);
	}

	fclose(file);
	return 0;
}

iva_t state_get_iva(state_t *state, char *code)
{
	if (strlen(code) != 1)
		return IVA_NVAL;

	if (!hashtable_has_key(state->iva_map, (void*)(uint64_t)code[0]))
		return IVA_NVAL;

	return (iva_t)(uint64_t)hashtable_get(state->iva_map, (void*)(uint64_t)code[0]);
}

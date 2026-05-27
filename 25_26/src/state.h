#ifndef _STATE_H_
#define _STATE_H_

#include <stdbool.h>

#include <sus/hashtable.h>
#include <sus/ivector.h>

#include "structures.h"

typedef struct state
{
	//hashtable_t<&char, &iva_t>
	hashtable_t *iva_map;

	//hashtable_t<&ean_t, product_t*>
	hashtable_t *product_map;

	//TODO: Move to ll for better removal (actually test)
	//ivector_t<product_t*>
	ivector_t *product_vec;

	//hashtable_t<&ean_t, *basket_entry_t>
	hashtable_t *basket;

	//TODO: Review removal case: must not compact indices but may need the memory back (NULL the pointer?)
	//ivector_t<invoice_t*>
	ivector_t *invoices;
} state_t;

int state_init(state_t *state);
int state_load_iva(state_t *state, char *path);
iva_t state_get_iva(state_t *state, char *code);

bool state_product_in_basket(state_t *state, ean_t ean);
void state_add_update_product(state_t *state, product_t *prod);

#endif

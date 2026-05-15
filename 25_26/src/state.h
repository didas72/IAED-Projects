#ifndef _STATE_H_
#define _STATE_H_

#include <sus/hashtable.h>
#include <sus/ivector.h>

#include "structures.h"

typedef struct state
{
	//hashtable_t<&ean_t, product_t*>
	hashtable_t *product_map;

	//TODO: Move to ll for better removal (actually test)
	//ivector_t<product_t*>
	ivector_t *product_vec;

	basket_t *basket;

	//TODO: Review removal case: must not compact indices but may need the memory back (NULL the pointer?)
	//ivector_t<invoice_t*>
	ivector_t *invoices;
} state_t;

#endif

#ifndef _STRUCTURES_H_
#define _STRUCTURES_H_

#include <stdint.h>

#include <sus/ivector.h>
#include <sus/ivector_utils.h>

// 8 or 13 decimal digits (with check)
// BCD is 6.5B (uint64_t)
typedef uint_fast64_t ean_t;
#define EAN_NVAL ((ean_t)~0)

// Cents
typedef uint_fast32_t price_t;

// Percentage
typedef uint_fast8_t iva_t;
#define IVA_NVAL ((iva_t)~0)

// Units
typedef uint_fast32_t quantity_t;

// 9 decimal digits
// Binary is ~30 bits (uint32_t)
typedef uint_fast32_t nif_t;

typedef struct product
{
	ean_t ean;
	quantity_t stock;
	quantity_t total_sold;
	iva_t iva;
	price_t price;
	char *description;
} product_t;

typedef struct basket_entry
{
	product_t *product;
	quantity_t quantity;
} basket_entry_t;

typedef struct basket
{
	//ivector_t<basket_entry_t*>
	ivector_t *entries;
} basket_t;

typedef struct invoice
{
	nif_t nif;
	char *client_name;
} invoice_t;

#endif

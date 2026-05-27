#include "structures.h"

#include <stdlib.h>

#include <sus/hashes.h>

#include "errs.h"

product_t *product_create(ean_t ean, quantity_t stock, iva_t iva, price_t price, char *desc)
{
	product_t *prod = malloc(sizeof(product_t));

	CHECK_ALLOC(prod);

	prod->ean = ean;
	prod->stock = stock;
	prod->total_sold = 0;
	prod->iva = iva;
	prod->price = price;
	prod->description = desc;

	return prod;
}

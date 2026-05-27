#include "commands.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sus/ivector_utils.h>

#include "structures.h"
#include "ean.h"

static bool handle_p(ivector_t *parts, state_t *state);

bool handle_command(ivector_t *parts, state_t *state)
{
	char *command = *(char**)ivector_get(parts, 0);

	if (strlen(command) != 1)
	{
		printf("Bad command '%s'\n", command);
		return false;
	}

	char cmd = *command;

	switch (cmd)
	{
	case 'q':
		return false;

	case 'p':
		handle_p(parts, state);
		break;

	default:
		printf("Unknown command '%c'\n", cmd);
		return false;
	}

	return true;
}

static bool handle_p(ivector_t *parts, state_t *state)
{
	size_t argc = ivector_get_count(parts) - 1;
	char **argv = (char**)ivector_as_pointer(parts) + 1;

	if (argc < 5)
	{
		printf("Insufficient arguments for p\n");
		return false;
	}

	ean_t ean = ean_parse(argv[0]);
	if (ean == EAN_NVAL)
	{
		printf("invalid ean\n");
		return false;
	}

	if (state_product_in_basket(state, ean))
	{
		printf("product in use\n");
		return false;
	}

	iva_t iva = state_get_iva(state, argv[1]);
	if (iva == IVA_NVAL)
	{
		printf("invalid iva\n");
		return false;
	}

	price_t price;
	if (sscanf(argv[2], "%lu", &price) != 1 || price == 0)
	{
		printf("invalid price\n");
		return false;
	}

	quantity_t quantity;
	if (sscanf(argv[3], "%lu", &quantity) != 1)
	{
		printf("invalid quantity\n");
		return false;
	}

	size_t desc_len = argc - 5;
	for (size_t i = 5; i < argc; ++i)
		desc_len += strlen(argv[i]);

	char *desc = malloc(desc_len + 1);
	for (size_t i = 5; i < argc; ++i)
	{
		strcat(desc, argv[i]);
		desc[strlen(desc)] = ' ';
	}

	product_t *prod = product_create(ean, iva, quantity, price, desc);
	state_add_update_product(state, prod);

	return true;
}

#include <stdio.h>
#include <stdbool.h>

#include "state.h"
#include "cli.h"
#include "commands.h"

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("Usage: %s <iva_file>\n", argv[0]);
		return 1;
	}

	state_t state;
	state_init(&state);

	if (state_load_iva(&state, argv[1]) != 0)
	{
		printf("Could load IVA from '%s'.\n", argv[1]);
		return 1;
	}

	bool should_exit = false;

	while (!should_exit)
	{
		ivector_t *vec = get_command();

		if (ivector_get_count(vec) == 0)
			break;

		if (!handle_command(vec))
			should_exit = true;
	}

	return 0;
}

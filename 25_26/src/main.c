#include <stdio.h>
#include <stdbool.h>

#include "cli.h"
#include "commands.h"

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	bool should_exit = false;

	while (!should_exit)
	{
		ivector_t *vec = get_command();

		if (ivector_get_count(vec) == 0)
			break;

		if (!handle_command(vec))
			should_exit = true;

		ivector_destroy(vec);
	}

	return 0;
}

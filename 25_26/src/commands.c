#include "commands.h"

#include <stdio.h>
#include <string.h>

bool handle_command(ivector_t *parts)
{
	char *command = ivector_get(parts, 0);

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

	default:
		printf("Unknown command '%c'\n", cmd);
		return false;
	}

	return true;
}

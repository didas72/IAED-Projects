#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "state.h"
#include "commands.h"

#define CMD_MAX 65535
#define ARG_MAX 16384

size_t read_cmd(char *buff, size_t len, char **parts)
{ //Pray we dont overflow parts
	fgets(buff, len, stdin);

	size_t i = ~(size_t)0;
	char *start = buff;
	char *head = buff;
	char in_quotes = 0;

	while (*head)
	{
		switch (*head)
		{
			case ' ':
				if (in_quotes)
					break;
				*head = 0;
				if (head != start)
					parts[++i] = start;
				start = head + 1;
				break;
				
			case '"':
				if (!in_quotes)
				{
					start = head + 1;
					in_quotes = 1;
					break;
				}
				*head = 0;
				parts[++i] = start;
				start = head + 1;
				in_quotes = 0;
				break;

			case '\n':
				*head = 0; //Ignore everything and just save this, should only be at end
				break;
		}

		++head;
	}

	if (head != start && *start)
		parts[++i] = start;

	return i + 1;
}

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	char buff[CMD_MAX];
	char *parts[ARG_MAX];

	state_t *state = state_create(LOCALE_EN);

	char should_run = 1;
	while (should_run)
	{
		size_t partc = read_cmd(buff, CMD_MAX, parts);
#ifdef DEBUG
		if (!partc)
		{
			fprintf(stderr, "Nothing read\n");
			should_run = 0;
			continue;
		}
		if (!parts[0][0] || parts[0][1])
		{
			fprintf(stderr, "First part of command must be one character long\n");
		}
#endif
		
		char cmd = parts[0][0];
		char **cmd_argv = &parts[1];
		size_t cmd_argc = partc - 1;

		switch (cmd)
		{
			case 'q':
				should_run = 0;
				break;
			
			case 'c':
				cmd_create(state, cmd_argv, cmd_argc);
				break;

			case 'l':
				cmd_list(state, cmd_argv, cmd_argc);
				break;

			case 'a':
				cmd_apply(state, cmd_argv, cmd_argc);
				break;

			case 'u':
				cmd_user(state, cmd_argv, cmd_argc);
				break;

			case 't':
				cmd_time(state, cmd_argv, cmd_argc);
				break;

			case 'r':
				cmd_remove(state, cmd_argv, cmd_argc);
				break;

			case 'd':
				fprintf(stderr, "Not implemented\n");
				should_run = 0;
				break;
		}
	}

	state_destroy(state);

	return 0;
}

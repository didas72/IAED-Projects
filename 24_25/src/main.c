#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

	char should_run = 1;
	while (should_run)
	{
		size_t partc = read_cmd(buff, CMD_MAX, parts);
#ifdef DEBUG
		if (!partc)
		{
			should_run = 0;
			continue;
		}
		if (!parts[0][0] || parts[0][1])
		{
			fprintf(stderr, "First part of command must be one character long\n");
			abort();
		}
#endif
		
		switch (parts[0][0])
		{
			case 'q':
				should_run = 0;
				break;
			
			case 'c':
			case 'l':
			case 'a':
			case 'r':
			case 'd':
			case 'u':
			case 't':
				fprintf(stderr, "Not implemented\n");
				should_run = 0;
				break;
		}
	}

	return 0;
}

#include <stdio.h>
#include <ctype.h>

size_t read_cmd(char *buff, size_t len, char **parts, size_t count)
{
	(void)count; //TODO: Not
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

	printf("Hey\n");

	char buff[1024];
	char *parts[16];
	size_t partc = read_cmd(buff, 1024, parts, 16);

	printf("Got %lu parts\n", partc);
	for (size_t i = 0; i < partc; ++i)
		printf("Part %lu is '%s'\n", i, parts[i]);

	return 0;
}

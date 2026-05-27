#include <stdio.h>
#include <stdbool.h>

#include <sus/ivector.h>

static char line_buffer[64*1024];

ivector_t *get_command()
{
	bool quotes = false;
	int ch, head = 0;
	char *first = line_buffer;
	//ivector_t<char*>
	ivector_t *vec = ivector_create(sizeof(char*));

	while ((ch = getchar()) != EOF && ch != '\n')
	{
		if (ch == ' ' && !quotes)
		{
			if (first == &line_buffer[head])
				continue;

			line_buffer[head++] = 0;
			ivector_append(vec, &first);
			first = &line_buffer[head];
			continue;
		}

		if (ch == '"')
		{
			if (!quotes)
			{
				quotes = true;
			}
			else
			{
				quotes = false;
				line_buffer[head++] = 0;
				ivector_append(vec, &first);
				first = &line_buffer[head];
			}
			continue;
		}

		line_buffer[head++] = ch;
	}

	//Append if haven't last arg
	if (first != &line_buffer[head])
	{
		line_buffer[head] = 0;
		ivector_append(vec, &first);
	}

	return vec;
}

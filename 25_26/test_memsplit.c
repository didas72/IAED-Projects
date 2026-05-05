#include "memsplit.h"

#include <stdio.h>

int main()
{
	memsplit_t *mem = memsplit_create();

	memsplit_insert(mem, (void *)0x200, 0x200);
	printf("%p\n", memsplit_query(mem, (void*)0x0));
	printf("%p\n", memsplit_query(mem, (void*)0x100));
	printf("%p\n", memsplit_query(mem, (void*)0x200));
	printf("%p\n", memsplit_query(mem, (void*)0x300));
	printf("%p\n", memsplit_query(mem, (void*)0x399));
	printf("%p\n", memsplit_query(mem, (void*)0x400));
	printf("%p\n", memsplit_query(mem, (void*)0x500));
}

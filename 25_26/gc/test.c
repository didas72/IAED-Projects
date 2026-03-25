#include <stdio.h>

#include "cgc.h"

void *f1()
{
	int *a = &((int*)gcg_malloc(4*sizeof(int)))[2];
	int *b = gcg_malloc(32);
	return a;
}

int main()
{
	gcg_init();
	int *a = f1();
	gcg_collect();
}

#include <stdio.h>

#include "cgc.h"

void *f1()
{
	int *p1 = &((int*)cgc_malloc(4*sizeof(int)))[2];
	int *p2 = cgc_malloc(32);
	return p1;
}

int main()
{
	cgc_init();
	int *a = f1();
	cgc_collect();
}

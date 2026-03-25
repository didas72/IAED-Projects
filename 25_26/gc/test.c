#include <stdio.h>

#include "cgc.h"

void *f1()
{
	int **p1 = cgc_malloc(4*sizeof(int*));
	int *p2 = cgc_malloc(32);
	int *p3 = cgc_malloc(sizeof(int));
	p1[1] = p3;
	return &p1[2];
}

int main()
{
	int *a = f1();
	cgc_collect();
}

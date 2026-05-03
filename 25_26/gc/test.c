#include <stdio.h>
#include <stdlib.h>

#include "cgc.h"

void *f1()
{
	int **p1 = malloc(4*sizeof(int*));
	int *p2 = malloc(32); // Should get GCd after return
	int *p3 = malloc(sizeof(int));
	p1[1] = p3;
	return &p1[2];
}

#define N 2
int main()
{
	int *a = f1();
	printf("\n\n");
	void **ptrs = malloc(N*sizeof(void*)+0x2000);

	for (int i = 0; i < N; ++i)
	{
		printf("iter %d\n", i);
		ptrs[i] = malloc(16);
	}
}

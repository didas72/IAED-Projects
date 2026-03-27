#include <stdio.h>

#include "cgc.h"

void *f1()
{
	int **p1 = cgc_malloc(4*sizeof(int*));
	int *p2 = cgc_malloc(32); // Should get GCd after return
	int *p3 = cgc_malloc(sizeof(int));
	p1[1] = p3;
	return &p1[2];
}

#define N 2
int main()
{
	int *a = f1();
	printf("\n\n");
	void **ptrs = cgc_malloc(N*sizeof(void*)+0x2000);

	for (int i = 0; i < N; ++i)
	{
		printf("iter %d\n", i);
		ptrs[i] = cgc_malloc(16);
	}
}

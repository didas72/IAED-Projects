#include <stdio.h>

#include "cgc.h"

void f1()
{
	int *a = gcg_malloc(4*sizeof(int));
}

int main()
{
	gcg_collect();
}

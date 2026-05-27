#ifndef _ERRS_H_
#define _ERRS_H_

#include <stdlib.h>
#include <stdio.h>

#define CHECK_ALLOC(ptr) do { if (ptr == NULL) { printf("No memory.\n"); exit(0); } } while (0)

#endif

#ifndef _CGC_H_
#define _CGC_H_

#include <stddef.h>

void cgc_init();
void *cgc_malloc(size_t size);
void *cgc_free(void *ptr);
void cgc_collect();

#endif

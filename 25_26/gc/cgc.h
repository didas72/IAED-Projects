#ifndef _CGC_H_
#define _CGC_H_

#include <stddef.h>

void gcg_init();
void *gcg_malloc(size_t size);
void *gcg_free(void *ptr);
void gcg_collect();

#endif

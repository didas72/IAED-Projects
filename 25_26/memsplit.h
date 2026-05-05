#ifndef _MEMSPLIT_H_
#define _MEMSPLIT_H_

#include <stddef.h>

// Bits per level
#define MEMSPLIT_BPL 4
#define MEMSPLIT_ENTRIES (1 << MEMSPLIT_BPL)
#define MEMSPLIT_MASK (MEMSPLIT_ENTRIES - 1)

#define MEMSPLIT_DEPTH ((sizeof(ptrdiff_t) * 8) / MEMSPLIT_BPL)

typedef struct memsplit_t memsplit_t;

struct memsplit_t
{
	memsplit_t *children[MEMSPLIT_ENTRIES];
};

memsplit_t *memsplit_create();
void memsplit_destroy(memsplit_t *mem);

void memsplit_insert(memsplit_t *mem, void *allocation, size_t size);
void memsplit_remove(memsplit_t *mem, void *allocation);
void *memsplit_query(memsplit_t *mem, void *address);

void memsplit_dump(memsplit_t *mem);

#endif

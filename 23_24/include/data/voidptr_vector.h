#pragma once

#include <stddef.h>

#define VOIDPTRARR_DEFAULT_CAP 4

typedef struct
{
	void **data;
	size_t capacity;
	size_t count;
} voidptr_vector_t;

voidptr_vector_t *voidptr_vector_create();
void voidptr_vector_destroy(voidptr_vector_t *vec);
voidptr_vector_t *voidptr_vector_duplicate(voidptr_vector_t *vec);
char voidptr_vector_ensure(voidptr_vector_t *vec, size_t capacity);
char voidptr_vector_trim(voidptr_vector_t *vec);
char voidptr_vector_append(voidptr_vector_t *vec, void *data);
size_t voidptr_vector_get_count(voidptr_vector_t *vec);
char voidptr_vector_pop_back(voidptr_vector_t *vec);
char voidptr_vector_remove_at(voidptr_vector_t *vec, size_t index);
voidptr_vector_t *voidptr_vector_get_all(voidptr_vector_t *vec, int (*match)(void *));
size_t voidptr_vector_remove_all(voidptr_vector_t *vec, int (*match)(void *));
voidptr_vector_t *voidptr_vector_sort(voidptr_vector_t *vec, int (*comparer)(void *, void *));

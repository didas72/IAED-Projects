#include "data/voidptr_vector.h"

#include <stddef.h>
#include <string.h>

#ifndef USE_ALLOC_CHECK
#define USE_STANDARD_MEM
#endif
#include "alloc_check/alloc_check.h"



voidptr_vector_t *voidptr_vector_create()
{
	voidptr_vector_t *ret = CHKD_MALLOC(sizeof(voidptr_vector_t));
	if (!ret) return NULL;

	ret->data = CHKD_MALLOC(VOIDPTRARR_DEFAULT_CAP * sizeof(void *));
	if (!ret->data) { CHKD_FREE(ret); return NULL; }

	ret->count = 0;
	ret->capacity = VOIDPTRARR_DEFAULT_CAP;

	return ret;
}

void voidptr_vector_destroy(voidptr_vector_t *vec)
{
	CHKD_FREE(vec->data);
	CHKD_FREE(vec);
}

voidptr_vector_t *voidptr_vector_duplicate(voidptr_vector_t *vec)
{
	voidptr_vector_t *ret = voidptr_vector_create();
	if (!ret)
		return NULL;

	if (voidptr_vector_ensure(ret, vec->count))
	{
		voidptr_vector_destroy(ret);
		return NULL;
	}

	for (size_t i = 0; i < vec->count; i++)
		voidptr_vector_append(ret, vec->data[i]);

	return ret;
}

char voidptr_vector_ensure(voidptr_vector_t *vec, size_t capacity)
{
	if (vec->capacity >= capacity) return 0;

	if (vec->capacity < VOIDPTRARR_DEFAULT_CAP) vec->capacity = VOIDPTRARR_DEFAULT_CAP;
	while (vec->capacity < capacity) vec->capacity <<= 1;

	void **tmp = CHKD_REALLOC(vec->data, vec->capacity * sizeof(void *));
	if (!tmp) return 1;

	vec->data = tmp;
	return 0;
}

char voidptr_vector_trim(voidptr_vector_t *vec)
{
	if (vec->count == vec->capacity) return 0;

	void **tmp = CHKD_REALLOC(vec->data, vec->count * sizeof(void *));
	if (!tmp) return 1;

	vec->data = tmp;
	vec->capacity = vec->count;
	return 0;
}

char voidptr_vector_append(voidptr_vector_t *vec, void *data)
{
	if (voidptr_vector_ensure(vec, vec->count + 1)) return 1;
	vec->data[vec->count++] = data;
	return 0;
}

size_t voidptr_vector_get_count(voidptr_vector_t *vec)
{
	return vec->count;
}

char voidptr_vector_pop_back(voidptr_vector_t *vec)
{
	if (!vec->count)
		return 1;
	
	vec->count--;
	return 0;
}

char voidptr_vector_remove_at(voidptr_vector_t *vec, size_t index)
{
	if (index >= vec->count)
		return 1;

	memmove(&vec->data[index], &vec->data[index + 1], (vec->count - index - 1) * sizeof(void *));
	vec->count--;
	return 0;
}

voidptr_vector_t *voidptr_vector_get_all(voidptr_vector_t *vec, int (*match)(void *))
{
	voidptr_vector_t *ret = voidptr_vector_create();

	for (size_t i = 0; i < vec->count; i++)
		if (match(vec->data[i]))
			voidptr_vector_append(ret, vec->data[i]);

	return ret;
}

size_t voidptr_vector_remove_all(voidptr_vector_t *vec, int (*match)(void *))
{
	size_t counter = 0;

	for (size_t i = vec->count - 1; i < ~(size_t)0; i--)
	{
		if (match(vec->data[i]))
		{
			voidptr_vector_remove_at(vec, i);
			counter++;
		}
	}

	return counter;
}

voidptr_vector_t *voidptr_vector_sort(voidptr_vector_t *vec, int (*comparer)(void *, void *))
{
	for (size_t gap = vec->count / 2; gap > 0; gap >>= 1)
	{
		for (size_t i = gap; i < vec->count; i++)
		{
			void *tmp = vec->data[i];

			size_t j;
			for (j = i; j >= gap && comparer(vec->data[j - gap], tmp) > 0; j -= gap)
				vec->data[j] = vec->data[j - gap];

			vec->data[j] = tmp;
		}
	}

	return vec;
}

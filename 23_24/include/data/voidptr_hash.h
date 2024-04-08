#pragma once

#include <stddef.h>

#include "data/voidptr_vector.h"



#define VOIDPTRHASH_DEFAULT_CAP 64



typedef struct voidptr_hash_entry_t voidptr_hash_entry_t;

typedef struct
{
	voidptr_hash_entry_t **entries;
	size_t capacity;
	size_t count;
	size_t (*hasher)(void*);
	int (*comparer)(void*, void*);
} voidptr_hash_t;

struct voidptr_hash_entry_t
{
	voidptr_hash_entry_t *next;
	void *content;
	void *key;
};



voidptr_hash_t *voidptr_hash_create(size_t (*hasher)(void*), int (*comparer)(void*, void*));
void voidptr_hash_destroy(voidptr_hash_t *table);
char voidptr_hash_add(voidptr_hash_t *table, void *key, void *value);
void *voidptr_hash_get(voidptr_hash_t *table, void *key);
char voidptr_hash_remove(voidptr_hash_t *table, void *key, void **removed_key, void **removed_content);
voidptr_vector_t *voidptr_hash_list_keys(voidptr_hash_t *table);
voidptr_vector_t *voidptr_hash_list_contents(voidptr_hash_t *table);
char voidptr_hash_resize(voidptr_hash_t *table, size_t capacity);

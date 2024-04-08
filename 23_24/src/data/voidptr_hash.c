#include "data/voidptr_hash.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifndef USE_ALLOC_CHECK
#define USE_STANDARD_MEM
#endif
#include "alloc_check/alloc_check.h"

#include "data/voidptr_vector.h"



voidptr_hash_t *voidptr_hash_create(size_t (*hasher)(void*), int (*comparer)(void*, void*))
{
	voidptr_hash_t *table = CHKD_MALLOC(sizeof(voidptr_hash_t));
	if (!table) return NULL;

	table->entries = CHKD_MALLOC(sizeof(voidptr_hash_entry_t*) * VOIDPTRHASH_DEFAULT_CAP);
	if (!table->entries) { CHKD_FREE(table); return NULL; }
	
	memset(table->entries, 0, sizeof(voidptr_hash_entry_t*) * VOIDPTRHASH_DEFAULT_CAP);

	table->capacity = VOIDPTRHASH_DEFAULT_CAP;
	table->count = 0;
	table->hasher = hasher;
	table->comparer = comparer;

	return table;
}

void voidptr_hash_destroy(voidptr_hash_t *table)
{
	voidptr_hash_entry_t *entry, *tmp;

	for (size_t i = 0; i < table->capacity; i++)
	{
		entry = table->entries[i];
		
		while (entry)
		{
			tmp = entry;
			entry = entry->next;
			CHKD_FREE(tmp);
		}
	}

	CHKD_FREE(table->entries);
	CHKD_FREE(table);
}

char voidptr_hash_add(voidptr_hash_t *table, void *key, void *value)
{
	if (table->count > table->capacity >> 1)
	{
		voidptr_hash_resize(table, table->capacity << 2);
		//ignore failure, still able to proceed
	}

	size_t hash_index = table->hasher(key) % table->capacity;
	voidptr_hash_entry_t *last_root = table->entries[hash_index];

	voidptr_hash_entry_t *new_entry = CHKD_MALLOC(sizeof(voidptr_hash_entry_t));
	if (!new_entry) return 1;

	new_entry->content = value;
	new_entry->key = key;
	new_entry->next = last_root;
	table->entries[hash_index] = new_entry;
	table->count++;

	return 0;
}

void *voidptr_hash_get(voidptr_hash_t *table, void *key)
{
	size_t hash_index = table->hasher(key) % table->capacity;
	voidptr_hash_entry_t *entry = table->entries[hash_index];

	if (entry == NULL)
		return NULL;

	while (entry->next != NULL && table->comparer(key, entry->key))
		entry = entry->next;

	return !table->comparer(key, entry->key) ? entry->content : NULL;
}

char voidptr_hash_remove(voidptr_hash_t *table, void *key, void **removed_key, void **removed_content)
{
	size_t hash_index = table->hasher(key) % table->capacity;
	voidptr_hash_entry_t *entry = table->entries[hash_index], **ptr_store = &table->entries[hash_index];

	if (entry == NULL)
		return 1;

	while (entry->next != NULL && table->comparer(key, entry->key))
	{
		ptr_store = &entry->next;
		entry = entry->next;
	}

	if (table->comparer(key, entry->key))
		return 1;
	
	if (removed_key) *removed_key = entry->key;
	if (removed_content) *removed_content = entry->content;
	*ptr_store = entry->next;
	CHKD_FREE(entry);
	return 0;
}

voidptr_vector_t *voidptr_hash_list_keys(voidptr_hash_t *table)
{
	voidptr_vector_t *ret = voidptr_vector_create();
	if (!ret) return NULL;

	if (voidptr_vector_ensure(ret, table->count))
	{
		voidptr_vector_destroy(ret);
		return NULL;
	}

	for (size_t i = 0; i < table->capacity; i++)
	{
		voidptr_hash_entry_t *entry = table->entries[i];

		while (entry)
		{
			voidptr_vector_append(ret, entry->key);
			entry = entry->next;
		}
	}

	return ret;
}

voidptr_vector_t *voidptr_hash_list_contents(voidptr_hash_t *table)
{
	voidptr_vector_t *ret = voidptr_vector_create();
	if (!ret) return NULL;

	if (voidptr_vector_ensure(ret, table->count))
	{
		voidptr_vector_destroy(ret);
		return NULL;
	}

	for (size_t i = 0; i < table->capacity; i++)
	{
		voidptr_hash_entry_t *entry = table->entries[i];

		while (entry)
		{
			voidptr_vector_append(ret, entry->content);
			entry = entry->next;
		}
	}

	return ret;
}

char voidptr_hash_resize(voidptr_hash_t *table, size_t capacity)
{
	if (table->capacity == capacity)
		return 0;

	voidptr_vector_t *keys = voidptr_hash_list_keys(table);
	if (!keys) return 1;
	voidptr_vector_t *values = voidptr_hash_list_contents(table);
	if (!values) { voidptr_vector_destroy(keys); return 1; }

	voidptr_hash_entry_t *entry, *prev;

	for (size_t i = 0; i < table->capacity; i++)
	{
		entry = table->entries[i];
		
		while (entry)
		{
			prev = entry;
			entry = entry->next;
			CHKD_FREE(prev);
		}
	}

	voidptr_hash_entry_t **tmp = CHKD_MALLOC(capacity * sizeof(voidptr_hash_entry_t*));
	if (!tmp) { voidptr_vector_destroy(keys); voidptr_vector_destroy(values); return 1; }

	memset(tmp, 0, sizeof(voidptr_hash_entry_t*) * capacity);

	CHKD_FREE(table->entries);
	table->entries = tmp;
	table->capacity = capacity;
	table->count = 0;

	for (size_t i = 0; i < keys->count; i++)
		voidptr_hash_add(table, keys->data[i], values->data[i]);

	voidptr_vector_destroy(keys);
	voidptr_vector_destroy(values);

	return 0;
}


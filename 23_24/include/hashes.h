#pragma once

#include <stddef.h>

size_t hash_str(void *ptr);
int compare_str(void *ptr1, void *ptr2);

size_t hash_timestamp(void *ptr);
int compare_timestamp(void *ptr1, void *ptr2);

#pragma once

#include "structs.h"

#define DIV_CEIL(a, b) ((((a) + (b) - 1) / (b)))
#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)



int get_args(char ***argv, int *argc);

void set_movent_park_match_park(park_t *ptr);
//HACKY: Relies on static global
int movement_park_match(void *ptr);

int park_id_comparer(void *ptr1, void *ptr2);

char validate_license_plate(char *plate);

char *my_strdup(char *str);

double determine_cost(movement_t *movement);

int movement_compare(void *ptr1, void *ptr2);
int billing_day_compare(void *ptr1, void *ptr2);
int park_compare(void *ptr1, void *ptr2);

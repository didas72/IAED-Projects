#include "main.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#ifndef USE_ALLOC_CHECK
#define USE_STANDARD_MEM
#endif
#include "alloc_check/alloc_check.h"

#include "errs.h"
#include "aux.h"
#include "structs.h"

#define MAX_PARKS 20



static full_state_t *state;



int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	#ifdef USE_ALLOC_CHECK
	atexit(&cleanup_alloc_checks);
	atexit(&report_alloc_checks);
	#endif

	state = full_state_create();
	char should_exit = 0;

	while (!should_exit)
		should_exit = handle_command();

	full_state_destroy(state);

	exit(0);
}



char handle_command()
{
	char **argv;
	int argc;

	if (get_args(&argv, &argc)) return 1;

	if (argc < 1) //no command
		return 0;
		
	if (strlen(argv[0]) != 1) //multi-leter command
		return 2;

	switch (*argv[0])
	{
		case 'q':
			if (argc == 1) return handle_quit();
			else return 3; //invalid argc

		case 'p':
			if (argc == 6) return handle_create_park(argv[1], argv[2], argv[3], argv[4], argv[5]);
			else if (argc == 1) return handle_list_parks();
			else return 3; //invalid argc

		case 'e':
			if (argc == 5) return handle_entry(argv[1], argv[2], argv[3], argv[4]);
			else return 3; //invalid argc

		case 's':
			if (argc == 5) return handle_exit(argv[1], argv[2], argv[3], argv[4]);
			else return 3; //invalid argc

		case 'v':
			if (argc == 2) return handle_list_vehicle_movements(argv[1]);
			else return 3; //invalid argc

		case 'f':
			if (argc == 2) return handle_show_full_billing(argv[1]);
			else if (argc == 3) return handle_show_day_billing(argv[1], argv[2]);
			else return 3; //invalid argc

		case 'r':
			if (argc == 2) return handle_remove_park(argv[1]);
			else return 3; //invalid argc

		default:
			return 4; //invalid command
	}
}

char handle_quit()
{
	return 1;
}

char handle_create_park(char *name, char *capacity, char *cost_f15, char *cost_n15, char *cost_max)
{
	if (voidptr_hash_get(state->parks, name))
	{
		printf("%s: parking already exists.\n", name);
		return 0;
	}
	
	size_t cap = strtoul(capacity, NULL, 10);
	double c_f15 = strtod(cost_f15, NULL),
		c_n15 = strtod(cost_n15, NULL),
		c_m = strtod(cost_max, NULL);

	if (cap == 0)
	{
		printf(ERR_NVAL_CAPACITY, capacity);
		return 0;
	}
	if (c_f15 <= 0 || c_n15 <= 0 || c_m <= 0 || 
		c_f15 > c_n15 || c_n15 > c_m)
	{
		printf(ERR_NVAL_COST);
		return 0;
	}
	if (state->parks->count >= MAX_PARKS)
	{
		printf(ERR_PARK_LIMIT);
		return 0;
	}
	
	park_t *park = park_create(name, cap, c_f15, c_n15, c_m);
	voidptr_hash_add(state->parks, park->name, park);
	return 0;
}

char handle_list_parks()
{
	voidptr_vector_t *vec = voidptr_hash_list_contents(state->parks);
	voidptr_vector_sort(vec, park_id_comparer);

	for (size_t i = 0; i < vec->count; i++)
	{
		park_t *park = vec->data[i];
		printf("%s %ld %ld\n", park->name, park->capacity, park->capacity - park->fill);
	}

	voidptr_vector_destroy(vec);
	return 0;
}

char handle_entry(char *name, char *plate, char *date, char *time)
{
	(void)time;

	park_t *park;
	vehicle_t *vehicle;
	timestamp_t now;
	movement_t *movement;

	if ((park = voidptr_hash_get(state->parks, name)) == NULL)
	{
		printf(ERR_PARK_NEXISTS, name);
		return 0;
	}
	if (park->fill == park->capacity)
	{
		printf(ERR_PARK_FULL, name);
		return 0;
	}
	if (validate_license_plate(plate))
	{
		printf(ERR_NVAL_PLATE, plate);
		return 0;
	}
	if ((vehicle = voidptr_hash_get(state->vehicles, plate)))
	{
		if (vehicle->cur_park)
		{
			printf(ERR_NVAL_ENTRY, plate);
			return 0;
		}
	}

	//HACKY: merge date and time
	date[strlen(date)] = ' ';
	now = timestamp_parse(date);
	if (now == NVAL_TIMESTAMP || timestamp_compare(now, state->last_time) < 0)
	{
		printf(ERR_NVAL_DATE);
		return 0;
	}

	if (!vehicle)
	{
		vehicle = vehicle_create(plate);
		voidptr_hash_add(state->vehicles, vehicle->plate, vehicle);
	}
	
	//Keep movement in vehicle only
	//Only add to park when exiting
	movement = movement_create(park, vehicle, now);
	voidptr_vector_append(vehicle->movements, movement);

	voidptr_hash_add(park->parked_vehicles, vehicle->plate, (void *)1);
	
	vehicle->cur_park = park;
	park->fill++;
	state->last_time = now;

	printf("%s %ld\n", name, park->capacity - park->fill);

	return 0;
}

char handle_exit(char *name, char *plate, char *date, char *time)
{
	(void)time;
	
	park_t *park;
	vehicle_t *vehicle;
	timestamp_t now, today;
	movement_t *movement;
	billing_day_t *billing_day;
	
	if ((park = voidptr_hash_get(state->parks, name)) == NULL)
	{
		printf(ERR_PARK_NEXISTS, name);
		return 0;
	}
	if (validate_license_plate(plate))
	{
		printf(ERR_NVAL_PLATE, plate);
		return 0;
	}
	if (!(vehicle = voidptr_hash_get(state->vehicles, plate)) || vehicle->cur_park != park)
	{
		printf(ERR_NVAL_EXIT, plate);
		return 0;
	}

	//HACKY: merge date and time
	date[strlen(date)] = ' ';
	now = timestamp_parse(date);
	today = timestamp_to_date(now);
	if (now == NVAL_TIMESTAMP || timestamp_compare(now, state->last_time) < 0)
	{
		printf(ERR_NVAL_DATE);
		return 0;
	}

	movement = vehicle->movements->data[vehicle->movements->count - 1];
	movement->exit = now;
	movement->cost = determine_cost(movement);

	if (!(billing_day = voidptr_hash_get(park->days, (void *)today)))
	{
		billing_day = billing_day_create(today);
		voidptr_hash_add(park->days, (void *)today, billing_day);
	}

	voidptr_vector_append(billing_day->movements, movement);
	billing_day->total += movement->cost;

	voidptr_hash_remove(park->parked_vehicles, vehicle->plate, NULL, NULL);

	vehicle->cur_park = NULL;
	park->fill--;
	state->last_time = now;

	printf("%s %s", plate, timestamp_format(movement->entry)); //HACKY: Pay for your static buffer fuckeries
	printf(" %s %.2f\n", timestamp_format(movement->exit), movement->cost);

	return 0;
}

char handle_list_vehicle_movements(char *plate)
{
	vehicle_t *vehicle;

	if (validate_license_plate(plate))
	{
		printf(ERR_NVAL_PLATE, plate);
		return 0;
	}
	if (!(vehicle = voidptr_hash_get(state->vehicles, plate)) ||
		vehicle->movements->count == 0)
	{
		printf(ERR_NO_ENTRIES, plate);
		return 0;
	}

	voidptr_vector_t *dup = voidptr_vector_duplicate(vehicle->movements);
	voidptr_vector_sort(dup, movement_compare);

	for (size_t i = 0; i < dup->count; i++)
	{
		movement_t *movement = dup->data[i];

		printf("%s", movement->park->name);
		printf(" %s", timestamp_format(movement->entry));
		if (movement->exit != NVAL_TIMESTAMP)
			printf(" %s", timestamp_format(movement->exit));
		putchar('\n');
	}

	voidptr_vector_destroy(dup);
	return 0;
}

char handle_show_full_billing(char *name)
{
	park_t *park;

	if (!(park = voidptr_hash_get(state->parks, name)))
	{
		printf(ERR_PARK_NEXISTS, name);
		return 0;
	}

	voidptr_vector_t *days = voidptr_hash_list_contents(park->days);
	voidptr_vector_sort(days, billing_day_compare);

	for (size_t i = 0; i < days->count; i++)
	{
		billing_day_t *day = days->data[i];
		printf("%s %.2f\n", timestamp_format_date(day->date), day->total);
	}

	voidptr_vector_destroy(days);
	return 0;
}

char handle_show_day_billing(char *name, char* date)
{
	park_t *park;
	timestamp_t day;

	if (!(park = voidptr_hash_get(state->parks, name)))
	{
		printf(ERR_PARK_NEXISTS, name);
		return 0;
	}
	day = timestamp_parse_date(date);
	if (day == NVAL_TIMESTAMP || timestamp_compare(day, state->last_time) > 0)
	{
		printf(ERR_NVAL_DATE);
		return 0;
	}

	billing_day_t *billing_day = voidptr_hash_get(park->days, (void *)day);
	if (!billing_day)
		return 0;

	for (size_t i = 0; i < billing_day->movements->count; i++)
	{
		movement_t *movement = billing_day->movements->data[i];
		printf("%s %s %.2f\n", movement->vehicle->plate, timestamp_format_hour(movement->exit), movement->cost);
	}

	return 0;
}

char handle_remove_park(char *name)
{
	park_t *park;

	if (!(park = voidptr_hash_get(state->parks, name)))
	{
		printf(ERR_PARK_NEXISTS, name);
		return 0;
	}

	//No need to get content or key
	//Content is already in variable park
	//Key is park->name, removed by park_destroy
	voidptr_hash_remove(state->parks, name, NULL, NULL);
	park_destroy(park, state->vehicles);

	voidptr_vector_t *parks = voidptr_hash_list_contents(state->parks);
	voidptr_vector_sort(parks, park_compare);

	for (size_t i = 0; i < parks->count; i++)
	{
		park_t *park = parks->data[i];
		printf("%s\n", park->name);
	}

	voidptr_vector_destroy(parks);
	return 0;
}

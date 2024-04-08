#include "structs.h"

#include <stdio.h>
#include <string.h>

#ifndef USE_ALLOC_CHECK
#define USE_STANDARD_MEM
#endif
#include "alloc_check/alloc_check.h"

#include "hashes.h"
#include "aux.h"

#define MINS_PER_PERIOD 15
#define MINS_PER_HOUR 60
#define HRS_PER_DAY 24
#define DAYS_PER_YEAR 365
#define MINS_PER_DAY (MINS_PER_HOUR * HRS_PER_DAY)
#define MINS_PER_YEAR (MINS_PER_DAY * DAYS_PER_YEAR)



static int day_in_month[12] = 
	{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static timestamp_t minutes_at_month[12] = 
	{ 0, 44640, 84960, 129600, 172800, 217440, 260640, 305280, 349920, 393120, 437760, 480960 };

timestamp_t timestamp_parse(char *str)
{
	int day, year, month, hour, minute;
	int count = sscanf(str, "%d-%d-%d %d:%d",
		&day, &month, &year, &hour, &minute);

	if (count != 5) return NVAL_TIMESTAMP;
	if (year < 0 || year > 9999 || month < 1 || month > 12 ||
		day < 1 || day > day_in_month[month - 1] || 
		hour < 0 || hour > 23 || minute < 0 || minute > 59)
		return NVAL_TIMESTAMP;

	timestamp_t ret =
		(timestamp_t)minute
		+ (timestamp_t)hour * MINS_PER_HOUR
		+ ((timestamp_t)day - 1) * MINS_PER_DAY
		+ (timestamp_t)minutes_at_month[month - 1]
		+ (timestamp_t)year * MINS_PER_YEAR;

	return ret;
}
timestamp_t timestamp_parse_date(char *str)
{
	int day, year, month;
	int count = sscanf(str, "%d-%d-%d", &day, &month, &year);

	if (count != 3) return NVAL_TIMESTAMP;
	if (year < 0 || year > 9999 || month < 1 || month > 12 ||
		day < 1 || day > day_in_month[month - 1])
		return NVAL_TIMESTAMP;

	timestamp_t ret =
		((timestamp_t)day - 1) * MINS_PER_DAY
		+ (timestamp_t)minutes_at_month[month - 1]
		+ (timestamp_t)year * MINS_PER_YEAR;

	return ret;
}
timestamp_t timestamp_to_date(timestamp_t timestamp)
{
	return timestamp - timestamp % MINS_PER_DAY;
}

static char __format_timestamp_buffer[64];
char *timestamp_format(timestamp_t timestamp)
{
	snprintf(__format_timestamp_buffer, 63, "%02d-%02d-%04d %02d:%02d", 
		timestamp_get_day(timestamp) + 1,
		timestamp_get_month(timestamp) + 1, 
		timestamp_get_year(timestamp), 
		timestamp_get_hour(timestamp), 
		timestamp_get_minute(timestamp));
	return __format_timestamp_buffer;
}
char *timestamp_format_date(timestamp_t timestamp)
{
	snprintf(__format_timestamp_buffer, 63, "%02d-%02d-%04d", 
		timestamp_get_day(timestamp) + 1,
		timestamp_get_month(timestamp) + 1, 
		timestamp_get_year(timestamp));
	return __format_timestamp_buffer;
}
char *timestamp_format_hour(timestamp_t timestamp)
{
	snprintf(__format_timestamp_buffer, 63, "%02d:%02d", 
		timestamp_get_hour(timestamp), 
		timestamp_get_minute(timestamp));
	return __format_timestamp_buffer;
}
int timestamp_get_minute(timestamp_t timestamp)
{
	return (int)(timestamp % MINS_PER_HOUR);
}
int timestamp_get_hour(timestamp_t timestamp)
{
	return (int)((timestamp / MINS_PER_HOUR) % HRS_PER_DAY);
}
int timestamp_get_day(timestamp_t timestamp)
{
	size_t remainder = timestamp % MINS_PER_YEAR;

	for (int i = 11; i >= 0; i--)
	{
		if (remainder >= minutes_at_month[i])
			return (int)((remainder - minutes_at_month[i]) / MINS_PER_DAY);
	}

	return 0; //NOTE: Impossible to get here
}
int timestamp_get_month(timestamp_t timestamp)
{
	size_t remainder = timestamp % MINS_PER_YEAR;

	for (int i = 11; i >= 0; i--)
	{
		if (remainder >= minutes_at_month[i])
			return i;
	}

	return 0; //NOTE: Impossible to get here
}
int timestamp_get_year(timestamp_t timestamp)
{
	return (int)(timestamp / MINS_PER_YEAR);
}
int timestamp_compare(timestamp_t first, timestamp_t second)
{
	return first < second ? -1 : (first > second ? 1 : 0);
}

timespan_t timestamp_get_elapsed(timestamp_t begin, timestamp_t end)
{
	if (begin > end)
		return NVAL_TIMESPAN;
	
	return end - begin;
}
int timespan_get_periods(timespan_t span)
{
	return DIV_CEIL(span % MINS_PER_DAY, MINS_PER_PERIOD);
}
int timespan_get_hours(timespan_t span)
{
	return span / MINS_PER_HOUR;
}
int timespan_get_days(timespan_t span)
{
	return span / MINS_PER_DAY;
}

movement_t *movement_create(park_t *park, vehicle_t *vehicle, timestamp_t entry)
{
	movement_t *ret = CHKD_MALLOC(sizeof(movement_t));
	if (!ret) return NULL;

	ret->park = park;
	ret->vehicle = vehicle;
	ret->entry = entry;
	ret->exit = NVAL_TIMESTAMP;
	ret->cost = 0.0;

	return ret;
}
void movement_destroy(movement_t *movement)
{
	CHKD_FREE(movement);
}

billing_day_t *billing_day_create(timestamp_t date)
{
	billing_day_t *ret = CHKD_MALLOC(sizeof(billing_day_t));
	if (!ret) return NULL;

	ret->movements = voidptr_vector_create();
	if (!ret->movements) { CHKD_FREE(ret); return NULL; }

	ret->date = date;
	ret->total = 0.0;

	return ret;
}
void billing_day_destroy(billing_day_t *day)
{
	voidptr_vector_destroy(day->movements);
	CHKD_FREE(day);
}

static size_t park_id = 0;
park_t *park_create(char *name, size_t capacity, double cost_f15, double cost_n15, double cost_max)
{
	park_t *ret = CHKD_MALLOC(sizeof(park_t));
	if (!ret) return NULL;

	ret->name = CHKD_MALLOC(strlen(name) + 1);
	if (!ret->name) { CHKD_FREE(ret); return NULL; }

	ret->days = voidptr_hash_create(hash_timestamp, compare_timestamp);
	if (!ret->days) { CHKD_FREE(ret->name); CHKD_FREE(ret); return NULL; }

	ret->parked_vehicles = voidptr_hash_create(hash_str, compare_str);
	if (!ret->parked_vehicles) { voidptr_hash_destroy(ret->days); CHKD_FREE(ret->name); CHKD_FREE(ret); return NULL; }

	strcpy(ret->name, name);
	ret->capacity = capacity;
	ret->fill = 0;
	ret->cost_f15 = cost_f15;
	ret->cost_n15 = cost_n15;
	ret->cost_max = cost_max;
	ret->id = park_id++;

	return ret;
}
void park_destroy(park_t *park, voidptr_hash_t *vehicles)
{
	voidptr_vector_t *vec;

	vec = voidptr_hash_list_keys(park->parked_vehicles);
	for (size_t i = 0; i < vec->count; i++)
	{
		char *plate = vec->data[i];
		vehicle_t *vehicle = voidptr_hash_get(vehicles, plate);
		vehicle_remove_park(vehicle, park);
	}
	voidptr_vector_destroy(vec);

	vec = voidptr_hash_list_contents(park->days);
	for (size_t i = 0; i < vec->count; i++)
	{
		billing_day_t *day = vec->data[i];
		for (size_t j = 0; j < day->movements->count; j++)
		{
			movement_t *mov = day->movements->data[j];
			vehicle_remove_park(mov->vehicle, park);
			movement_destroy(mov);
		}
		billing_day_destroy(day);
	}
	voidptr_vector_destroy(vec);

	//keys don't need freeing, they are void* cast timestamp_t's

	CHKD_FREE(park->name);
	voidptr_hash_destroy(park->days);
	voidptr_hash_destroy(park->parked_vehicles);
	CHKD_FREE(park);
}

vehicle_t *vehicle_create(char *plate)
{
	vehicle_t *ret = CHKD_MALLOC(sizeof(vehicle_t));
	if (!ret) return NULL;

	ret->plate = CHKD_MALLOC(strlen(plate) + 1);
	if (!ret->plate) { CHKD_FREE(ret); return NULL; }

	ret->movements = voidptr_vector_create();
	if (!ret->movements) { CHKD_FREE(ret->plate); CHKD_FREE(ret); return NULL; }

	strcpy(ret->plate, plate);
	ret->cur_park = NULL;
	ret->last_removed_park = NULL;

	return ret;
}
void vehicle_destroy(vehicle_t *vehicle)
{
	for (size_t i = 0; i < vehicle->movements->count; i++)
		movement_destroy(vehicle->movements->data[i]);

	voidptr_vector_destroy(vehicle->movements);
	CHKD_FREE(vehicle->plate);
	CHKD_FREE(vehicle);
}
void vehicle_remove_park(vehicle_t *vehicle, park_t *park)
{
	if (vehicle->last_removed_park == park)
		return;

	vehicle->last_removed_park = park;

	movement_t *last_movement = vehicle->movements->data[vehicle->movements->count - 1];

	if (last_movement->park == park && last_movement->exit == NVAL_TIMESTAMP)
	{
		voidptr_vector_pop_back(vehicle->movements);
		movement_destroy(last_movement);
	}

	set_movent_park_match_park(park);
	voidptr_vector_remove_all(vehicle->movements, movement_park_match);

	if (vehicle->cur_park == park)
		vehicle->cur_park = NULL;
}

full_state_t *full_state_create()
{
	full_state_t *ret = CHKD_MALLOC(sizeof(full_state_t));
	if(!ret) return NULL;

	ret->vehicles = voidptr_hash_create(hash_str, compare_str);
	if (!ret->vehicles) { CHKD_FREE(ret); return NULL; }

	ret->parks = voidptr_hash_create(hash_str, compare_str);
	if (!ret->parks) { CHKD_FREE(ret->vehicles); CHKD_FREE(ret); return NULL; }

	ret->last_time = 0;

	return ret;
}
void full_state_destroy(full_state_t *state)
{
	voidptr_vector_t *vec;
	
	vec = voidptr_hash_list_contents(state->parks);
	for (size_t i = 0; i < vec->count; i++)
		park_destroy(vec->data[i], state->vehicles);
	voidptr_vector_destroy(vec);

	//No need to free state->parks keys, used pointer to park->name

	vec = voidptr_hash_list_contents(state->vehicles);
	for (size_t i = 0; i < vec->count; i++)
		vehicle_destroy(vec->data[i]);
	voidptr_vector_destroy(vec);

	//No need to free state->vehicles keys, used pointer to vehicle->plate

	voidptr_hash_destroy(state->vehicles);
	voidptr_hash_destroy(state->parks);

	CHKD_FREE(state);
}

#pragma once

#include <stddef.h>

#include "data/voidptr_vector.h"
#include "data/voidptr_hash.h"



typedef size_t timestamp_t;
typedef size_t timespan_t;

#define NVAL_TIMESTAMP (timestamp_t)~0
#define NVAL_TIMESPAN (timespan_t)~0



typedef struct
{
	double total;
	timestamp_t date;
	//vector<movement_t>
	voidptr_vector_t *movements;
} billing_day_t;

typedef struct
{
	char *name;
	size_t capacity;
	size_t fill;
	double cost_f15, cost_n15, cost_max;
	//unordered_map<&timestamp_t, billing_day_t>
	voidptr_hash_t *days;
	//unordered_map<char *, (void *)1>
	voidptr_hash_t *parked_vehicles;

	size_t id;
} park_t;

typedef struct
{
	char *plate;
	park_t *cur_park;
	//vector<movement_t>
	voidptr_vector_t *movements;

	//For cleanup optimizations
	park_t *last_removed_park;
} vehicle_t;

typedef struct
{
	park_t *park;
	vehicle_t *vehicle;
	timestamp_t entry, exit;
	double cost;
} movement_t;

typedef struct
{
	timestamp_t last_time;
	//unordered_map<char *, vehicle_t>
	voidptr_hash_t *vehicles;
	//unordered_map<char *, park_t>
	voidptr_hash_t *parks;
} full_state_t;



timestamp_t timestamp_parse(char *str);
timestamp_t timestamp_parse_date(char *str);
timestamp_t timestamp_to_date(timestamp_t timestamp);
char *timestamp_format(timestamp_t timestamp);
char *timestamp_format_date(timestamp_t timestamp);
char *timestamp_format_hour(timestamp_t timestamp);
int timestamp_get_minute(timestamp_t timestamp);
int timestamp_get_hour(timestamp_t timestamp);
int timestamp_get_day(timestamp_t timestamp);
int timestamp_get_month(timestamp_t timestamp);
int timestamp_get_year(timestamp_t timestamp);
//sign(first - second)
int timestamp_compare(timestamp_t first, timestamp_t second);

timespan_t timestamp_get_elapsed(timestamp_t begin, timestamp_t end);
int timespan_get_periods(timespan_t span);
int timespan_get_hours(timespan_t span);
int timespan_get_days(timespan_t span);

movement_t *movement_create(park_t *park, vehicle_t *vehicle, timestamp_t entry);
void movement_destroy(movement_t *movement);

billing_day_t *billing_day_create(timestamp_t date);
void billing_day_destroy(billing_day_t *day);

park_t *park_create(char *name, size_t capacity, double cost_f15, double cost_n15, double cost_max);
void park_destroy(park_t *park, voidptr_hash_t *vehicles);

vehicle_t *vehicle_create(char *plate);
void vehicle_destroy(vehicle_t *vehicle);
void vehicle_remove_park(vehicle_t *vehicle, park_t *park);

full_state_t *full_state_create();
void full_state_destroy(full_state_t *state);

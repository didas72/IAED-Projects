#include "structs.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MIN_YEAR 2025
#define DAYS_IN_YEAR 365
#define MONTHS_IN_YEAR 12

#define VACCINE_NAME_MAX 50


char parse_batch(char *str, batch_t *batch)
{
	char *hex = "0123456789ABCDEF";
	memset(batch, 0, sizeof(batch_t));
	unsigned char i;

	for (i = 0; i < (BATCH_LEN * 2) && str[i]; ++i)
	{
		char *chr = strchr(hex, str[i]);
		if (!chr)
			return 1;
		int idx = chr - hex;
		batch->data[i >> 1] |= idx << ((1 - (i & 1)) << 2);
	}

	batch->data[BATCH_LEN] = i;

	return str[i];
}
void print_batch(batch_t batch)
{
	char *hex = "0123456789ABCDEF";
	for (int i = 0; i < batch.len; ++i)
		putchar(hex[(batch.data[i >> 1] >> ((1 - (i & 1)) << 2)) & 0x0F]);
}
size_t batch_hasher(void *batch)
{
	size_t hash = 0;

	for (int i = 0; i < BATCH_LEN; ++i)
		hash = hash * 733 + *((char*)batch + i); //Use random prime

	return hash;
}
int batch_comprarer(void *first, void *second)
{
	batch_t *a = first, *b = second;

	for (int i = 0; i < BATCH_LEN && i < a->len && i < b->len; i++)
	{
		if (a->data[i] == b->data[i]) continue;
		return a->data[i] - b->data[i];
	}

	return a->len - b->len;
}

date_t parse_date(char *str)
{
	int days_in_month[MONTHS_IN_YEAR] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int days_at_month[MONTHS_IN_YEAR] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

	int day, month, year;
	sscanf(str, "%d%*c%d%*c%d", &day, &month, &year); //NOTE: Accepts any separator character, ignores failed scans

	if (year < MIN_YEAR || month < 1 || month > MONTHS_IN_YEAR || day < 1 || day > days_in_month[month-1])
		return DATE_NVAL;

	return (date_t)(day-1) + days_at_month[month - 1] + (year - MIN_YEAR) * DAYS_IN_YEAR; //NOTE: Does not handle leap years
}
void print_date(date_t date)
{
	date_t days_at_month[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

	int day, month, year;

	year = date / DAYS_IN_YEAR;
	date %= DAYS_IN_YEAR;
	for (month = MONTHS_IN_YEAR - 1; days_at_month[month] > date; month--);
	day = date - days_at_month[month];

	printf("%02d-%02d-%04d", day + 1, month + 1, year + MIN_YEAR);
}

char invalid_vaccine_name(char *name)
{
	if (strlen(name) > VACC_NAME_MAX)
		return 1;
	
	if (strpbrk(name, " \t\n") != NULL)
		return 1;

	return 0;
}

vaccine_t *vaccine_create(batch_t batch, date_t expiration_date, int count, char *name)
{
	vaccine_t *vaccine = malloc(sizeof(vaccine_t));
	if (!vaccine)
		return NULL;

	vaccine->batch = batch;
	vaccine->expiration_date = expiration_date;
	vaccine->count = count;
	strncpy(vaccine->name, name, VACC_NAME_MAX);
	vaccine->name[VACC_NAME_MAX] = 0;

	return vaccine;
}
//#define vaccine_destroy(vaccine) do { free(vaccine); } while (0)

inoculation_t *inoculation_create(batch_t batch, date_t date, char *name)
{
	inoculation_t *inoc = malloc(sizeof(inoculation_t));
	if (!inoc)
		return NULL;

	inoc->batch = batch;
	inoc->date = date;
	inoc->name = malloc(strlen(name) + 1);
	if (!inoc->name)
	{ free(inoc); return NULL; }
	strcpy(inoc->name, name);

	return inoc;
}
void inoculation_destroy(inoculation_t *inoc)
{
	free(inoc->name);
	free(inoc);
}

void print_vaccine(vaccine_t *vaccine, int times_applied)
{
	fputs(vaccine->name, stdout);
	putchar(' ');
	print_batch(vaccine->batch);
	putchar(' ');
	print_date(vaccine->expiration_date);
	printf(" %d %d", vaccine->count, times_applied);
}

void print_inoculation(inoculation_t *inoc)
{
	fputs(inoc->name, stdout);
	putchar(' ');
	print_batch(inoc->batch);
	putchar(' ');
	print_date(inoc->date);
}

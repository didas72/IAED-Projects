#include "structs.h"

#include <string.h>
#include <stdio.h>



char parse_batch(char *str, batch_t *batch)
{
	char *hex = "0123456789ABCDEF";
	memset(batch, 0, sizeof(batch_t));
	int i;

	for (i = 0; i < 20 && str[i]; i++)
	{
		char *chr = strchr(hex, str[i]);
		if (!chr)
			return 1;
		int idx = chr - hex;
		batch->data[i >> 1] |= idx << ((i & 1) << 2);
	}

	return (i == 20 && str[20]) ? 1 : 0;
}

void print_batch(batch_t batch)
{ //REVIEW: Print how much of it? Always 20 chars?
	char *hex = "0123456789ABCDEF";
	for (int i = 0; i < 20; i++)
		putchar(hex[batch.data[i >> 1] & (0x0F << ((i & 1) << 2))]);
}



#define MIN_YEAR 2025
#define DAYS_IN_YEAR 365
#define MONTHS_IN_YEAR 12

date_t parse_date(char *str)
{
	int days_in_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int days_at_month[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

	int day, month, year;
	sscanf(str, "%d%*c%d%*c%d", &day, &month, &year); //NOTE: Accepts any separator character, ignores failed scans

	if (year < MIN_YEAR || month < 1 || month > 12 || day < 1 || day > days_in_month[month])
		return DATE_NVAL;

	return (date_t)(day-1) + days_at_month[month - 1] + (year - MIN_YEAR) * DAYS_IN_YEAR; //NOTE: Does not handle leap years
}

void print_date(date_t date)
{
	int days_at_month[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

	int day, month, year;

	year = date / DAYS_IN_YEAR;
	date %= DAYS_IN_YEAR;
	for (month = MONTHS_IN_YEAR - 1; days_at_month[month] > date; month--);
	day = date - days_at_month[month];

	printf("%02d-%02d-%04d", day + 1, month + 1, year + MIN_YEAR);
}



void print_vaccine(vaccine_t *vaccine, int times_applied)
{
	fputs(stdout, vaccine->name);
	putchar(' ');
	print_batch(vaccine->batch);
	putchar(' ');
	print_date(vaccine->expiration_date);
	printf(" %d %d", vaccine->count - times_applied, times_applied);
}



void print_inoculation(inoculation_t *inoc)
{
	fputs(stdout, inoc->name);
	putchar(' ');
	print_batch(inoc->batch);
	putchar(' ');
	print_date(inoc->date);
}

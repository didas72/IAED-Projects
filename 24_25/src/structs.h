#ifndef STRUCTS_H_
#define STRUCTS_H_

#include <stddef.h>

#define BATCH_LEN 10

#define VACC_NAME_MAX 50

#define DATE_NVAL (~(date_t)0)



typedef long unsigned int date_t;

typedef struct batch_t
{ //REVIEW: Sorting might not benefit from this format (reorder hex chars if needed?)
	unsigned char data[BATCH_LEN];
	unsigned char len;
} batch_t;

typedef struct vaccine_t
{
	batch_t batch;
	date_t expiration_date;
	int available;
	int applied;
	char name[VACC_NAME_MAX + 1];
} vaccine_t;

typedef struct inoculation_t
{
	batch_t batch;
	date_t date;
	char *name;
} inoculation_t;



char parse_batch(char *str, batch_t *batch);
void print_batch(batch_t batch);
size_t batch_hasher(void *batch);
int batch_comprarer(void *first, void *second);

date_t parse_date(char *str);
void print_date(date_t date);

char invalid_vaccine_name(char *name);

vaccine_t *vaccine_create(batch_t batch, date_t expiration_date, int available, char *name);
#define vaccine_destroy(vaccine) do { free(vaccine); } while (0)

inoculation_t *inoculation_create(batch_t batch, date_t date, char *name);
void inoculation_destroy(inoculation_t *inoc);

void print_vaccine(vaccine_t *vaccine);

void print_inoculation(inoculation_t *inoc);

#endif

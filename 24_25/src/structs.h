#ifndef STRUCTS_H_
#define STRUCTS_H_

#define BATCH_LEN 10

#define VACC_NAME_MAX 50

#define DATE_NVAL (~(date_t)0)



typedef long unsigned int date_t;

typedef struct batch_t
{ //REVIEW: Sorting might not benefit from this format (reorder hex chars if needed?)
	unsigned char data[BATCH_LEN];
} batch_t;

typedef struct vaccine_t
{
	batch_t batch;
	date_t expiration_date;
	//Number of vaccines produced in batch (readonly)
	int count;
	char name[VACC_NAME_MAX + 1];
} vaccine_t;

typedef struct inoculation_t
{
	batch_t batch;
	date_t date;
	char *name;
} inoculation_t;



batch_t parse_batch(char *str, batch_t *batch);
void print_batch(batch_t batch);

date_t parse_date(char *str);
void print_date(date_t date);

void print_vaccine(vaccine_t *vaccine, int times_applied);

void print_inoculation(inoculation_t *inoc);

#endif

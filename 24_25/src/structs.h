#ifndef STRUCTS_H_
#define STRUCTS_H_

#define BATCH_LEN 10

#define VACC_NAME_MAX 50

typedef long unsigned int date_t;

typedef struct batch_t
{
	unsigned char data[BATCH_LEN];
} batch_t;

typedef struct vaccine_t
{
	batch_t batch;
	date_t expiration_date;
	int count;
	char name[VACC_NAME_MAX + 1];
} vaccine_t;

typedef struct inoculation_t
{
	batch_t batch;
	char *name;
} inoculation_t;

#endif

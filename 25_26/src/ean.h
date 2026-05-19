#ifndef _EAN_H_
#define _EAN_H_

#include "structures.h"

ean_t ean_parse(char *str);

#define ean_print(ean) do { printf("%lx", ean); } while(0);

#endif

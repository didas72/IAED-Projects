#ifndef ERROR_UTILS_H_
#define ERROR_UTILS_H_

#define ERR_NO_MEMORY 0
#define ERR_MAX_VACCINES 1
#define ERR_BATCH_DUP 2
#define ERR_BATCH_NVAL 3
#define ERR_NAME_NVAL 4
#define ERR_DATE_NVAL 5
#define ERR_COUNT_NVAL 6
#define ERR_NO_VACCINE 7
#define ERR_DEPLETED 8
#define ERR_VACCINATED 9
#define ERR_NO_BATCH 10
#define ERR_NO_USER 11
#define ERR_MAX 12

#define ERR(num) do { puts(state->error_locales[num]); } while (0)
#define ERR_ARG(num, arg) do { printf(state->error_locales[num], arg); putchar('\n'); } while (0)

#endif

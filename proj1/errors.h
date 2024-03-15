/*
 * File: errors.h
 * Author: Diogo Diniz (ist1106196)
 * Description: Error functions and macros (header)
 * Notes: Has no associated .c file
*/

#ifndef ERRORS_H
#define ERRORS_H

/*===OPTION MACROS===*/
/*Undefine to output errors properly*/
/*#define USE_SMARTER_ERROR_OUTPUT_STREAM*/

/*===MACROS===*/
#ifdef USE_SMARTER_ERROR_OUTPUT_STREAM
#define ERR_OUTPUT_STREAM stderr
#else
#define ERR_OUTPUT_STREAM stdout
#endif

/*Error string literals*/
#define ERR_STOP_EXISTS "%s: stop already exists.\n"
#define ERR_STOP_N_EXISTS "%s: no such stop.\n"
#define ERR_ROUTE_N_EXISTS "%s: no such line.\n"
#define ERR_LINK_N_ASSOC "link cannot be associated with bus line.\n"
#define ERR_NEG_VAL "negative cost or duration.\n"
#define ERR_INV_SORT "incorrect sort option.\n"

/*Print formatted errors to the selected stream*/
#define ERROR0(message) fprintf(ERR_OUTPUT_STREAM, message);
#define ERROR1(format, arg1) fprintf(ERR_OUTPUT_STREAM, format, arg1);
#define ERROR2(format, arg1, arg2) fprintf(ERR_OUTPUT_STREAM, format, arg1, arg2);
#define ERROR3(format, arg1, arg2, arg3) fprintf(ERR_OUTPUT_STREAM, format, arg1, arg2, arg3);

#endif

#ifndef COMMANDS_H_
#define COMMANDS_H_

#include "state.h"

void cmd_create(state_t *state, char **argv, size_t argc);
void cmd_list(state_t *state, char **argv, size_t argc);
void cmd_apply(state_t *state, char **argv, size_t argc);
void cmd_time(state_t *state, char **argv, size_t argc);

#endif

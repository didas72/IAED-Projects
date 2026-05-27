#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include <stdbool.h>

#include <sus/ivector.h>

#include "state.h"

bool handle_command(ivector_t *parts, state_t *state);

#endif

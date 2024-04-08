#pragma once

#include <stddef.h>



char handle_command();

char handle_quit();
char handle_create_park(char *name, char *capacity, char *cost_f15, char *cost_n15, char *cost_max);
char handle_list_parks();
char handle_entry(char *name, char *plate, char *date, char *time);
char handle_exit(char *name, char *plate, char *date, char *time);
char handle_list_vehicle_movements(char *plate);
char handle_show_full_billing(char *name);
char handle_show_day_billing(char *name, char* date);
char handle_remove_park(char *name);

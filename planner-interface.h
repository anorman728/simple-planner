#ifndef plannerinterface_h
#define plannerinterface_h

#include "date-functions.h"

// Constants

#define PLANNER_INTERFACE__OK 0
#define PLANNER_INTERFACE__DB_ERROR 1
#define PLANNER_INTERFACE__IO_ERROR 2

// Functions
char planner_interface_initialize(char *filename);

char planner_interface_display_week(Date dayObj);

#endif

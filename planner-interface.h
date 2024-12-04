#ifndef plannerinterface_h
#define plannerinterface_h

#include "date-functions.h"

// Constants

#define PLANNER_INTERFACE__OK 0
#define PLANNER_INTERFACE__DB_ERROR 1
#define PLANNER_INTERFACE__IO_ERROR 2
#define PLANNER_INTERFACE__GENERAL_ERROR 3
// Not sure of better name.  This means that there's an error in the program
// itself, not with its interaction with interface or sqlite.

// Functions
char planner_interface_initialize(char *filename);

char planner_interface_display_week(Date dayObj);

#endif

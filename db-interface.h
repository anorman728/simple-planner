#ifndef dbinterface_h
#define dbinterface_h

#include "planner-functions.h"

// Constants.

#define DB_INTERFACE__OK            0
#define DB_INTERFACE__DB_ERROR      1
#define DB_INTERFACE__OUT_OF_MEMORY 2
#define DB_INTERFACE__CONT          3
#define DB_INTERFACE__PLANNER       4


// Functions.

char db_interface_initialize(char *filename);

char db_interface_finalize();

char db_interface_save(PlannerItem *item);

int db_interface_get_db_err();

char db_interface_build_err(char **str, int code);

void _db_interface_create_db_err();

char db_interface_get(PlannerItem **result, int id);

char db_interface_range(PlannerItem **result, Date lower, Date upper);

#endif

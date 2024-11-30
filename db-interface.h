#ifndef dbinterface_h
#define dbinterface_h

#include "planner-functions.h"

// Constants.

#define DB_INTERFACE__OK            0
#define DB_INTERFACE__DB_ERROR      1
#define DB_INTERFACE__OUT_OF_MEMORY 2
#define DB_INTERFACE__CONT          3
#define DB_INTERFACE__PLANNER       4
#define DB_INTERFACE__INTERNAL      5


// Functions.

char db_interface_initialize(char *filename);

char db_interface_finalize();

sqlite3 *db_interface_get_db();

char db_interface_save(PlannerItem *item);

char db_interface_update_desc(long id, char *newdesc);

char db_interface_delete(long id);

int db_interface_get_db_err();

char db_interface_build_err(char **str, int code);

void _db_interface_create_db_err();

char db_interface_get(PlannerItem **result, int id);

char db_interface_range(PlannerItem **result, Date lower, Date upper);

char db_interface_day(PlannerItem **result, Date date01);

#endif

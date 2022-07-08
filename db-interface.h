#ifndef dbinterface_h
#define dbinterface_h

#include "planner-functions.h"

// Constants.

const char DB_INTERFACE__OK;

const char DB_INTERFACE__DB_ERROR;

void db_interface_initialize(char *filename);

void db_interface_finalize();

void db_interface_save(PlannerItem *items[]);

int db_interface_get_db_err();

void db_interface_build_err(char **str, int code);

void _db_interface_create_db_err();

PlannerItem *db_interface_get(int id);

PlannerItem **db_interface_range(Date lower, Date upper);

#endif

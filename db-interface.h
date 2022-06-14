#ifndef dbinterface_h
#define dbinterface_h

#include "planner-functions.h"

void db_interface_initialize(char *filename);

void db_interface_finalize();

void db_interface_save(PlannerItem *items[], int amt);

PlannerItem *db_interface_get(int id);

PlannerItem **db_interface_range(Date lower, Date upper);

#endif

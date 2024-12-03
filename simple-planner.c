#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "planner-interface.h"
#include "date-functions.h"

#define ERR__GENERAL 1
#define ERR__MISSING_ARG 2

int main(int argc, char *argv[])
{
    if (argc == 1) {
        fprintf(stderr, "Must identify database file.\n");
        return ERR__MISSING_ARG;
    }

    char rc;

    if ((rc = planner_interface_initialize(argv[1]))) {
        return ERR__GENERAL;
    }

    time_t current_time;
    struct tm *time_info;

    time(&current_time);
    time_info = localtime(&current_time);
    Date today = tmToDate(*time_info);

    planner_interface_display_week(today);

    return 0;
}

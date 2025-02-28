#include <stdio.h>
#include <stdlib.h>

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

    planner_interface_display_week(todayDate());

    return 0;
}

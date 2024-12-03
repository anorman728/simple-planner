#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "planner-interface.h"

#include "date-functions.h"
#include "db-interface.h"
#include "planner-functions.h"

// No test for this module because it's basically only tested manually.

// Forward declaration of static functions.

static void printAllItemsInDay(Date dateObj);

static int appendItemMapping(int id);

static void resetItemMapping();

static void setRepType(char **repTypeStr, char rep);

// static variables.

/**
 * Currently-displayed number of item on screen.  Increments with every one
 * printed on the screen.
 */
static int displayKey = 0;

/**
 * Map of the displayed item number to the item id.
 */
static int *items = NULL;

/**
 * Store the current week displayed in memory.
 */
static Date *currentWeek = NULL;

/**
 * Initialize the db from the filename.
 *
 * @param   filename
 */
char planner_interface_initialize(char *filename)
{
    char rc;
    if ((rc = db_interface_initialize(filename))) {
        char *errStr;
        db_interface_build_err(&errStr, rc);
        printf("Database error: %s\n", errStr);
        free(errStr);
        return PLANNER_INTERFACE__DB_ERROR;
    }

    return PLANNER_INTERFACE__OK;
}

/**
 * Display the week that includes the specified day along with the options.
 *
 * @param   dayObj
 */
char planner_interface_display_week(Date dayObj)
{
    Date rollDay = getWeek(dayObj); // Starting today, rolls through the week.

    free(currentWeek);
    currentWeek = (Date *) malloc(sizeof(Date));
    memcpy(currentWeek, &rollDay, sizeof(rollDay));

    char *dayStr = NULL;
    resetItemMapping();

    char days[7] = {'S','M','T','W','R','F','A'};

    for (int i = 0; i < 7; i++) {
        toString(&dayStr, rollDay);
        printf("%c %s\n", days[i], dayStr);
        free(dayStr);
        printAllItemsInDay(rollDay);

        datepp(&rollDay);
    }

    return PLANNER_INTERFACE__OK;
}

// Static functions below this line.

static void printAllItemsInDay(Date dateObj)
{
    PlannerItem *item = NULL;
    char rc;

    while ((rc = db_interface_day(&item, dateObj)) == DB_INTERFACE__CONT) {
        char *repTypeStr = NULL;
        setRepType(&repTypeStr, item->rep);
        printf("  %d) %s%s\n", appendItemMapping(item->id), item->desc, repTypeStr);
        free(repTypeStr);
        freeItem(item);
    }
    printf("\n");

    if (rc != DB_INTERFACE__OK) {
        char *error = NULL;
        db_interface_build_err(&error, rc);
        printf("%s\n", error);
        free(error);
    }
}

/**
 * Append an id to the item mapping.
 */
static int appendItemMapping(int id)
{
    if (items == NULL) {
        items = (int *) malloc(sizeof(int));
    } else {
        items = (int *) realloc(items, sizeof(items) + sizeof(int));
    }

    items[displayKey] = id;

    return ++displayKey;
}

/**
 * Reset the item mapping, which maps the displayed number to the item id.
 */
static void resetItemMapping()
{
    free(items);
    items = NULL;
    displayKey = 0;
}

/**
 * Set the repetition type string to display.
 *
 * @param   repTypeStr  (String to set)
 * @param   rep
 */
static void setRepType(char **repTypeStr, char rep)
{
    char *repTypeStrDum;
    switch (rep) {
        case REP_YEARLY:
            repTypeStrDum = "yearly";
            break;
        default:
            *repTypeStr = (char *) malloc(1);
            strcpy(*repTypeStr, "");
            return;
    }

    *repTypeStr = (char *) malloc(strlen(repTypeStrDum) + 4);
    strcpy(*repTypeStr, " (");
    strcat(*repTypeStr, repTypeStrDum);
    strcat(*repTypeStr, ")");
}

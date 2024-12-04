#include <ctype.h>
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

static char showPrompt();

static char addItem();

static char getInput(char **inputStr, int len);

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

    printf("\n");

    for (int i = 0; i < 7; i++) {
        toString(&dayStr, rollDay);
        printf("%c %s\n", days[i], dayStr);
        free(dayStr);
        printAllItemsInDay(rollDay);

        datepp(&rollDay);
    }

    showPrompt();

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
        items = (int *) realloc(items, (displayKey + 1) * sizeof(int));
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

/**
 * Show the standard prompt.
 */
static char showPrompt()
{
    char rc = 0;
    printf("(A)dd, (E)dit, (D)elete, (P)revious, (N)ext, (C)urrent, (G)oto, (Q)uit\n> ");

    char *inp = NULL;
    if ((rc = getInput(&inp, 3))) { // 3 = first char, space, null term, I think.
        return rc;
    }
    char inpChar = tolower(inp[0]);
    free(inp);
    inp = NULL;

    switch (inpChar) {
        case 'a':
            return addItem();
        case 'c':
            return planner_interface_display_week(*currentWeek);
        case 'q':
            db_interface_finalize();
            // TODO: use print_db_err function, when it exists.
            printf("The sea was angry that day, my friends.  Like an old man trying to send back soup in a deli.\n");
            return PLANNER_INTERFACE__OK;
        default:
            printf("Select one of the parenthesized options.\n");
            showPrompt();
    }

    return PLANNER_INTERFACE__OK;
}

/**
 * Prompt for adding an item.
 *
 * @param   inputStr
 */
static char addItem()
{
    char rc;

    char *day = NULL;
    if ((rc = getInput(&day, 3))) { // 3 = day, newline, null terminator.
        return rc;
    }
    char dayChar = tolower(day[0]);
    free(day);
    day = NULL;


    PlannerItem *item;
    Date *dateDum = (Date *) malloc(sizeof(Date));

    memcpy(dateDum, currentWeek, sizeof(*currentWeek));

    char dateinc = 0;
    switch (dayChar) {
        // Skip sunday
        case 'm':
            dateinc = 1;
            break;
        case 't':
            dateinc = 2;
            break;
        case 'w':
            dateinc = 3;
            break;
        case 'r':
            dateinc = 4;
            break;
        case 'f':
            dateinc = 5;
            break;
        case 'a':
            dateinc = 6;
            break;
    }

    for (char i = 0; i < dateinc; i++) {
        datepp(dateDum);
    }

    printf("Description?\n");
    char *desc = NULL;
    if ((rc = getInput(&desc, 99))) {
        return rc;
    }

    printf("Repeat annually? (y/n)\n");
    char *repInp = NULL;
    if ((rc = getInput(&repInp, 3))) {
        return rc;
    }

    char rep = (tolower(repInp[0]) == 'y');
    free(repInp);
    repInp = NULL;

    if ((rc = buildItem(&item, 0, *dateDum, desc, rep))) {
        char *errMsg = NULL;
        planner_functions_build_err(&errMsg, rc);
        printf("%s\n", errMsg);
        free(errMsg);
        freeItem(item);
        free(dateDum);
        return PLANNER_INTERFACE__GENERAL_ERROR;
    }
    free(desc);
    desc = NULL;

    db_interface_save(item);
    // TODO: Use printDbErr function, when it exists.
    freeItem(item);
    free(dateDum);

    return planner_interface_display_week(*currentWeek);
}

/**
 * Get input from user and put into inputStr.  len is how much to pull from the
 * input.  Newlines are converted to null terminators.
 *
 * @param   inputStr
 * @param   len
 */
static char getInput(char **inputStr, int len)
{
    *inputStr = (char *) malloc(sizeof(char) * len);
    if (fgets(*inputStr, sizeof(char) * len, stdin) == NULL) {
        printf("IO error.\n");
        return PLANNER_INTERFACE__IO_ERROR;
    }

    int i = 0;
    while ((*inputStr)[i] != '\0' && i < len) {
        if ((*inputStr)[i] == '\n') {
            (*inputStr)[i] = '\0';
        }
        i++;
    }

    return PLANNER_INTERFACE__OK;
}

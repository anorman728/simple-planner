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

// I don't know if anybody is reading this as part of a hiring process, but
// just FYI, this is the last module I wrote, so it's sloppy because I'm
// getting antsy about actually using it.

// Also, I know the recursion risks a stack overflow, but... It's unlikely that
// I'll keep the program open that much and I kinda like recursion.  If it ever
// becomes a problem, I'll switch to a main loop.

static void printAllItemsInDay(Date dateObj);

static int appendItemMapping(long id);

static void resetItemMapping();

static void setRepType(char **repTypeStr, char rep);

static char showPrompt();

static char addItem();

static char editItem();

static char deleteItem();

static char gotoWeek();

static char getInput(char **inputStr, int len);

static void addFlashMessage(char *str);

static void displayFlashMessage();

static void printDbErr(char errCode);

// static variables.

/**
 * Currently-displayed number of item on screen.  Increments with every one
 * printed on the screen.
 */
static long displayKey = 0;

/**
 * Map of the displayed item number to the item id.
 */
static int *items = NULL;

/**
 * Store the current week displayed in memory.
 */
static Date *currentWeek = NULL;

/**
 * Message to display at end of current week.
 */
static char *flashMsg = NULL;

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
        errStr = NULL;
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

    // TODO: I may want to pass dayObj by refrence so I can find out if it's
    // pointing to the same place as currentWeek already is, thereby avoiding
    // freeing and reallocating something that's not changing.
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
        dayStr = NULL;
        printAllItemsInDay(rollDay);

        datepp(&rollDay);
    }

    displayFlashMessage();

    char rc;
    if ((rc = showPrompt()) == PLANNER_INTERFACE__CANCEL) {
        addFlashMessage("Canceled.\n");
        return planner_interface_display_week(*currentWeek);
    } else if (rc) {
        return rc;
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
        repTypeStr = NULL;
        freeItem(item);
        item = NULL;
    }
    printf("\n");

    if (rc != DB_INTERFACE__OK) {
        printDbErr(rc);
    }
}

/**
 * Append an id to the item mapping.
 */
static int appendItemMapping(long id)
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
        printf("Select one of the parenthesized options.\n");
        free(inp);
        return showPrompt();
    }
    char inpChar = tolower(inp[0]);
    free(inp);
    inp = NULL;

    switch (inpChar) {
        case 'a':
            return addItem();
        case 'e':
            return editItem();
        case 'd':
            return deleteItem();
        case 'g':
            return gotoWeek();
        case 'c':
            return planner_interface_display_week(*currentWeek);
        case 'q':
            if ((rc = db_interface_finalize())) {
                printDbErr(rc);
            }
            printf("The sea was angry that day, my friends.  Like an old man trying to send back soup in a deli.\n");
            return PLANNER_INTERFACE__OK;
        default:
            // I know this is code duplication, but atm I don't care much.
            printf("Select one of the parenthesized options.\n");
            return showPrompt();
    }
}

/**
 * Prompt for adding an item.
 */
static char addItem()
{
    char rc;

    char *day = NULL;
    if ((rc = getInput(&day, 3)) == PLANNER_INTERFACE__CANCEL) { // 3 = day, newline, null terminator.
        addFlashMessage("Usage like \"A T\" to add an item to Tuesday.\n");
        return rc;
    } else if (rc) {
        free(day);
        day = NULL;
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
        free(desc);
        desc = NULL;
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
        errMsg = NULL;
        freeItem(item);
        item = NULL;
        free(dateDum);
        dateDum = NULL;
        return PLANNER_INTERFACE__GENERAL_ERROR;
    }
    free(desc);
    desc = NULL;

    if ((rc = db_interface_save(item))) {
        printDbErr(rc);
    }
    freeItem(item);
    item = NULL;
    free(dateDum);
    dateDum = NULL;

    return planner_interface_display_week(*currentWeek);
}

/**
 * Prompt for editing an item.
 */
static char editItem()
{
    char rc;

    char *itemStr = NULL;
    if ((rc = getInput(&itemStr, 5)) == PLANNER_INTERFACE__CANCEL) { // 3 for items + 1 for line break + 1 null term = 5.
        free(itemStr);
        addFlashMessage("Usage like \"E 5\" to edit the fifth item displayed.\n");
        return rc;
    } else if (rc) {
        free(itemStr);
        itemStr = NULL;
        return rc;
    }

    long id = items[atoi(itemStr) - 1];
    free(itemStr);

    printf("New description?\n");
    char *desc = NULL;
    if ((rc = getInput(&desc, 99))) {
        free(desc);
        return rc;
    }

    if ((rc = db_interface_update_desc(id, desc))) {
        printDbErr(rc);
    }
    free(desc);

    return planner_interface_display_week(*currentWeek);
}

/**
 * Prompt for deleting an item.
 */
static char deleteItem()
{
    char rc;

    char *itemStr = NULL;
    if ((rc = getInput(&itemStr, 3)) == PLANNER_INTERFACE__CANCEL) {
        free(itemStr);
        itemStr = NULL;
        addFlashMessage("Usage like \"D 3\" to delete third item displayed.\n");
        return rc;
    } else if (rc) {
        free(itemStr);
        itemStr = NULL;
        return rc;
    }

    printf("Are you sure you want to delete item #%s? (y/n)\n", itemStr);
    char *confirmStr = NULL;
    if ((rc = getInput(&confirmStr, 3))) {
        free(confirmStr);
        confirmStr = NULL;
        free(itemStr);
        itemStr = NULL;
        return rc;
    }

    if (tolower(confirmStr[0]) != 'y') {
        free(confirmStr);
        confirmStr = NULL;
        return planner_interface_display_week(*currentWeek);
    }

    free(confirmStr);
    confirmStr = NULL;

    long id = items[atoi(itemStr) - 1];
    free(itemStr);

    if ((rc = db_interface_delete(id))) {
        printDbErr(rc);
    }

    return planner_interface_display_week(*currentWeek);
}

/**
 * Prompt for goto week.
 */
static char gotoWeek()
{
    char rc;
    char *weekStr = NULL;
    char *usageMsg = "Usage like \"G 241014\" to go to Oct 14, 2024.\n";
    if ((rc = getInput(&weekStr, 8)) == PLANNER_INTERFACE__CANCEL) {
        free(weekStr);
        weekStr = NULL;
        addFlashMessage(usageMsg);
        return rc;
    } else if (rc) {
        free(weekStr);
        weekStr = NULL;
        return rc;
    }

    if (strlen(weekStr) != 6) {
        free(weekStr);
        weekStr = NULL;
        addFlashMessage(usageMsg);
        return PLANNER_INTERFACE__CANCEL;
    }

    char dumStr[3];
    dumStr[2] = '\0';

    strncpy(dumStr, weekStr, 2);
    int yr = atoi(dumStr);

    strncpy(dumStr, weekStr + 2, 2);
    int mn = atoi(dumStr);

    strncpy(dumStr, weekStr + 4, 2);
    int dy = atoi(dumStr);

    Date newWeek = buildDate(yr - 1, mn - 1, dy - 1);
    free(weekStr);

    return planner_interface_display_week(newWeek);
}

/**
 * Get input from user and put into inputStr.  len is how much to pull from the
 * input.  Newlines are converted to null terminators.
 *
 * If including the *end* of user input, will want to take that into
 * consideration when deciding the len value, to make sure the newline doesn't
 * get sucked into the *next* time stdin is read.  It'll make for confusing
 * problems otherwise.
 *
 * @param   inputStr
 * @param   len
 */
static char getInput(char **inputStr, int len)
{
    *inputStr = (char *) malloc(sizeof(char) * len);
    if (fgets(*inputStr, sizeof(char) * len, stdin) == NULL) {
        printf("IO error.\n");
        free(*inputStr);
        *inputStr = NULL;
        return PLANNER_INTERFACE__IO_ERROR;
    }

    if ((*inputStr)[0] == '\n') {
        free(*inputStr);
        *inputStr = NULL;
        return PLANNER_INTERFACE__CANCEL;
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

/**
 * Add string to flash message.
 */
static void addFlashMessage(char *str)
{
    if (flashMsg == NULL) {
        flashMsg = (char *) malloc(strlen(str) + 2 * sizeof(char));
        strcpy(flashMsg, "");
    } else {
        flashMsg = (char *) realloc(flashMsg, strlen(str) + strlen(flashMsg) + 2 * sizeof(char));
    }
    strcat(flashMsg, str);
    strcat(flashMsg, "\n");
}

/**
 * Display flash message and flush it.
 */
static void displayFlashMessage()
{
    if (flashMsg == NULL) {
        return;
    }

    printf("%s\n", flashMsg);
    free(flashMsg);
    flashMsg = NULL;
}

/**
 * Print database error from return code.
 *
 * @param   errCode
 */
static void printDbErr(char errCode)
{
    char *error = NULL;
    db_interface_build_err(&error, errCode);
    printf("Database error: %s\n", error);
    free(error);
    error = NULL;
}

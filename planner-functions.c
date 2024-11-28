#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "planner-functions.h"
#include "date-functions.h"


/**
 * Build a planner item object.  Returns static as integer.
 *
 * @param   item    Pointer to pointer to point to new object.
 * @param   id      Id from db.  Zero if new object.
 * @param   dateObj Date object.
 * @param   desc    Description string.
 * @param   rep     Repetition, from header file's constants.
 * @return  int
 */
int buildItem(
    PlannerItem **item,
    long id,
    Date dateObj,
    char *desc,
    char rep
) {
    char *descHp = malloc(strlen(desc) + 1);

    if (descHp == NULL) {
        return PLANNER_STATUS__OUT_OF_MEMORY;
    }

    strcpy(descHp, desc);

    // Build item.
    PlannerItem *itmDum = malloc(sizeof(PlannerItem));

    if (itmDum == NULL) {
        return PLANNER_STATUS__OUT_OF_MEMORY;
    }

    itmDum->id     = id;
    itmDum->date   = dateObj;
    itmDum->desc   = descHp;
    itmDum->rep    = rep;

    *item = itmDum;

    return PLANNER_STATUS__OK;
}

/**
 * Destroy PlannerItem object.
 *
 * @param   *PlannerItem    Object to destroy.
 */
void freeItem(PlannerItem *obj)
{
    free(obj->desc);
    free(obj);
    obj = NULL; // (Almost) Always set freed pointers to null.
}

/**
 * Build error string from return code, for printing.
 *
 * The resulting string is on heap memory and must be freed!
 *
 * @param   str     SETS HEAP.  Pointer to string.
 * @param   code    Previously-returned error code.
 */
int planner_functions_build_err(char **str, int code)
{
    char *strdum;

    switch (code) {
        case PLANNER_STATUS__OK:
            strdum = "No error for planner functions.";
            break;
        case PLANNER_STATUS__OUT_OF_MEMORY:
            strdum = "Out of memory for planner functions.";
            break;
        default:
            strdum = "Unknown error."; // If this happens, something's wrong.
    }

    *str = malloc(strlen(strdum) + 1);

    if (str == NULL) {
        return PLANNER_STATUS__OUT_OF_MEMORY;
    }

    strcpy(*str, strdum);

    return PLANNER_STATUS__OK;
}

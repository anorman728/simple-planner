#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "planner-functions.h"

#include "date-functions.h"


// Constants

const char PLANNER_STATUS__OK = 0;

const char PLANNER_STATUS__OUT_OF_MEMORY = 1;


/**
 * Build a planner item object.  Returns static as integer.
 *
 * @param   item    Pointer to pointer to point to new object.
 * @param   id      Id from db.  Zero if new object.
 * @param   dateObj Date object.
 * @param   desc    Description string.
 * @param   rep     Repetition, from enum.
 * @param   exp     Expiry, year zero if none.
 * @param   done    "To-do" status.  (Described by struct doc.)
 * @return  int
 */
int buildItem(
    PlannerItem **item,
    long id,
    Date dateObj,
    char *desc,
    Repetition rep,
    Date exp,
    char done
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
    itmDum->exp    = exp;
    itmDum->rep    = rep;
    itmDum->done   = done;

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
 * Destroy array of PlannerItem objects *and* the array itself.
 *
 * @param   **PlannerItem   Objects to destroy.
 * @param   amt             Count of objects.
 */
void freeAll(PlannerItem **items)
{
    int i = 0;
    while (items[i] != NULL) {
        freeItem(items[i++]);
    }

    free(items);
    items = NULL; // Always set freed ponters to null.
}

#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "planner-functions.h"

#include "date-functions.h"

/**
 * Build a planner item object.
 *
 * @param   id      Id from db.  Zero if new object.
 * @param   dateObj Date object.
 * @param   desc    Description string.
 * @param   rep     Repetition, from enum.
 * @param   exp     Expiry, year zero if none.
 * @param   done    "To-do" status.  (Described by struct doc.)
 */
PlannerItem *buildItem(
    long id,
    Date dateObj,
    char *desc,
    Repetition rep,
    Date exp,
    char done
) {
    char *descHp = (char *) malloc(strlen(desc) + 1);
    strcpy(descHp, desc);

    // Build item.
    PlannerItem *item = (PlannerItem *) malloc(sizeof(PlannerItem));

    item->id    = id;
    item->date  = dateObj;
    item->desc  = descHp;
    item->exp   = exp;
    item->rep   = rep;
    item->done  = done;

    return item;
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
}

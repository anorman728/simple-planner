#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "planner-functions.h"

/**
 * Build a date from the year, month, and day.
 *
 * @param   yr  Year, count since 1900 (i.e., 2013 is 113).
 * @param   mn  Month, 0 - 11 (non-human-friendly version)
 * @param   dy  Day
 */
struct tm buildDate(int yr, int mn, int dy)
{
    // I considered putting the "human-friendly" version of these numbers here,
    // so user would put in 1-12 for months and something like 2013 instead of
    // 113 for the year, but I decided that that kind of conversion is best
    // handled exclusively by the frontend.

    struct tm returnVal = {};

    returnVal.tm_year = yr;
    returnVal.tm_mon = mn;
    returnVal.tm_mday = dy;

    returnVal.tm_hour = 0;
    returnVal.tm_min = 0;
    returnVal.tm_sec = 0;
    returnVal.tm_isdst = -1;
    // We're not concerned about anything more precise than the day, so DST just
    // gets in the way.

    mktime(&returnVal);

    return returnVal;
}

/**
 * Build a planner item object.
 *
 * @param   date    Date object, which should be built be buildDate
 * @param   desc    Description string.
 * @param   rep     Repetition, from enum.
 * @param   exp     Expiry, year zero if none.
 * @param   done    "To-do" status.  (Described by struct doc.)
 */
PlannerItem *buildItem(
    struct tm date,
    char *desc,
    Repetition rep,
    struct tm exp,
    char done
) {
    // Copy objects from stack to heap.

    // Date
    struct tm *dateHp = (struct tm *) malloc(sizeof(date));
    memcpy(dateHp, &date, sizeof(date));

    // Description
    char *descHp = (char *) malloc(strlen(desc) + 1);
    strcpy(descHp, desc);

    // Expiration
    struct tm *expHp = (struct tm *) malloc(sizeof(date));
    memcpy(expHp, &exp, sizeof(exp));


    // Repetition
    Repetition *repHp = (Repetition *) malloc(sizeof(Repetition));
    memcpy(repHp, &rep, sizeof(rep));
    // TODO: Don't use malloc for this.


    // Build item.
    PlannerItem *item = (PlannerItem *) malloc(sizeof(PlannerItem));

    item->date  = dateHp;
    item->desc  = descHp;
    item->exp   = expHp;

    item->rep   = repHp;
    item->done  = done;

    return item;
}

/**
 * Free PlannerItem without leaving dangling pointers.  Hopefully.
 *
 * @param   itm     The PlannerItem object to free.
 */
void freeItem(PlannerItem *itm)
{
    free(itm->date);
    free(itm->desc);
    free(itm->rep);
    free(itm->exp);

    free(itm);
}

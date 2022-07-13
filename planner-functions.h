#ifndef plannerfunctions_h
#define plannerfunctions_h

#include "date-functions.h"

// REMINDER: Never use memcpy with a PlannerItem object!  The char pointer
// screws that up.  If need to do that, then will need to make a special
// function in the same way that I needed to make a special function for freeing
// the object.

// Constants

const char PLANNER_STATUS__OK;

const char PLANNER_STATUS__OUT_OF_MEMORY;


typedef enum planner_repenum
{
    rep_D, // daily
    rep_W, // weekly
    rep_M, // monthly
    rep_Y, // yearly
    rep_N  // none
} Repetition;

typedef struct planner_itemstruct
{
    /** @var Id in database. */
    long id;

    /** @var Date event takes place. */
    Date date;

    /** @var Description. */
    char *desc;

    /** @var Day of expiration.  Jan 1, 2000 if none. */
    Date exp;

    /** @var Type of repetition. */
    Repetition rep;

    /** @var 0 if needs to be done, 1 if done, -1 if n/a. */
    short done;

} PlannerItem;

int buildItem(
    PlannerItem **item,
    long id,
    Date dateObj,
    char *desc,
    Repetition rep,
    Date exp,
    char done
);

void freeItem(PlannerItem *item);

void freeAll(PlannerItem **items);

int planner_functions_build_err(char **str, int code);


#endif

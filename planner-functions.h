#ifndef plannerfunctions_h
#define plannerfunctions_h

#include "date-functions.h"

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

PlannerItem *buildItem(
    long id,
    Date dateObj,
    char *desc,
    Repetition rep,
    Date exp,
    char done
);

void freeItem(PlannerItem *item);

#endif

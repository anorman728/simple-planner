#ifndef plannerfunctions_h
#define plannerfunctions_h

#include <time.h>

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

    /** @var Date object. */
    struct tm date;

    /** @var Description. */
    char *desc;

    /** @var Day of expiration.  Jan 1, 2000 if none. */
    struct tm exp;

    /** @var Type of repetition. */
    Repetition rep;

    /** @var 0 if needs to be done, 1 if done, -1 if n/a. */
    char done; // TODO: This needs to be changed to short!  I decided I shouldn't use chars like this.

} PlannerItem;

struct tm buildDate(int yr, int mn, int dy);

PlannerItem *buildItem(
    long id,
    struct tm date,
    char *desc,
    Repetition rep,
    struct tm exp,
    char done
);

void freeItem(PlannerItem *item);

#endif

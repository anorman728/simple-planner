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
    /** @var tm Date object. */
    struct tm *date;

    /** @var Description. */
    char *desc;

    /** @var Repetition Type of repetition. */
    Repetition *rep;

    /** @var int Day of expiration.  0 if none. */
    struct tm *exp;

} PlannerItem;

struct tm buildDate(int yr, int mn, int dy);

PlannerItem *buildItem(struct tm date, char *desc, Repetition rep, struct tm exp);

void freeItem(PlannerItem *item);

#endif

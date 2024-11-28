#ifndef plannerfunctions_h
#define plannerfunctions_h

#include "date-functions.h"

// REMINDER: Never use memcpy with a PlannerItem object!  The char pointer
// screws that up.  If need to do that, then will need to make a special
// function in the same way that I needed to make a special function for freeing
// the object.

// Constants

#define PLANNER_STATUS__OK              0
#define PLANNER_STATUS__OUT_OF_MEMORY   1

#define REP_NONE 0
#define REP_YEARLY 1


typedef struct planner_itemstruct
{
    /** @var Id in database. */
    long id;

    /** @var Date event takes place. */
    Date date;

    /** @var Description. */
    char *desc;

    /** @var Type of repetition, from constants. */
    char rep;

} PlannerItem;

int buildItem(
    PlannerItem **item,
    long id,
    Date dateObj,
    char *desc,
    char rep
);

void freeItem(PlannerItem *item);

int planner_functions_build_err(char **str, int code);


#endif

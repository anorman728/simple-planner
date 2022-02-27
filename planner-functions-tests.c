#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "planner-functions.h"

static void buildItemTest();

static PlannerItem *buildItemDummyFunction();

int sandboxing();

int main()
{
    //return sandboxing();

    // Run test on buildItem.
    buildItemTest();
}

/**
 * For piddling around with stuff.
 */
int sandboxing()
{
    struct tm *datedum = (struct tm *) malloc(sizeof(struct tm));
    datedum->tm_year = 105;
    datedum->tm_mon = 1;
    datedum->tm_mday = 6;
    mktime(datedum);

    free(datedum);
    
    return 0;
}

/**
 * Test the buildItem and destroyItem functions.
 *
 * @return  void
 */
static void buildItemTest()
{
    printf("Starting buildItemTest.\n");

    PlannerItem *itmDum = buildItemDummyFunction();

    if (itmDum->date->tm_mon != 6) {
        printf("FAILURE: date property is wrong.\n");
    }
    if (strcmp(itmDum->desc, "Planner description 436") != 0) {
        printf("FAILURE: desc property is wrong.\n");
    }
    if (*itmDum->rep != rep_W) {
        printf("FAILURE: rep property is wrong.\n");
    }
    if (itmDum->exp->tm_mon != 5) {
        printf("FAILURE: exp property is wrong.\n");
    }

    freeItem(itmDum);

    printf("Finished buildItemTest.\n");
}

// Helper functions below this line.

/**
 * Build a PlannerItem object.  This function exists to make sure that when the
 * passed values go out of scope in the stack, the pointers are still valid to
 * copies.
 *
 * @return PlannerItem
 */
static PlannerItem *buildItemDummyFunction()
{
    struct tm date = buildDate(103, 6, 1);
    struct tm exp = buildDate(103, 5, 1);

    return buildItem(date, "Planner description 436", rep_W, exp);
}

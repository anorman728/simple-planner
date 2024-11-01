#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "planner-functions.h"
#include "date-functions.h"

static void buildErrTest();
static void buildItemTest();

static PlannerItem *buildItemDummyFunction();

int main()
{
    buildErrTest();
    buildItemTest();
}

/**
 * Test planner_functions_build_err function.
 */
static void buildErrTest()
{
    printf("...Starting buildErrTest.\n");
    char *str;

    planner_functions_build_err(&str, PLANNER_STATUS__OK);

    if (strcmp(str, "No error for planner functions.") != 0) {
        printf("FAILURE: \"OK\" status not returned as expected.\n");
    }

    free(str);

    planner_functions_build_err(&str, PLANNER_STATUS__OUT_OF_MEMORY);

    if (strcmp(str, "Out of memory for planner functions.") != 0) {
        printf("FAILURE: Out of memory status not returned as expected. Found \"%s\".\n", str);
    }

    // Don't really need to test beyond this.

    free(str);
    printf("...Ending buildErrTest.\n");
}

/**
 * Test the buildItem and destroyItem functions.
 *
 * @return  void
 */
static void buildItemTest()
{
    printf("...Starting buildItemTest.\n");

    PlannerItem *itmDum = buildItemDummyFunction();

    if (itmDum->id != 0) {
        printf("FAILURE: id is wrong.\n");
    }
    if (itmDum->date.month != 6) {
        printf("FAILURE: date property is wrong.\n");
    }
    if (strcmp(itmDum->desc, "Planner description 436") != 0) {
        printf("FAILURE: desc property is wrong.\n");
    }
    if (itmDum->exp.month != 5) {
        printf("FAILURE: exp property is wrong.\n");
    }
    if (itmDum->rep != REP_YEARLY) {
        printf("FAILURE: rep property is wrong.\n");
    }
    if (itmDum->done != -1) {
        printf("FAILURE: done property is wrong.\n");
    }

    freeItem(itmDum);

    printf("...Finished buildItemTest.\n");
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
    Date dateObj = buildDate(103, 6, 1);
    Date exp = buildDate(103, 5, 1);

    PlannerItem *itm;

    int rc = buildItem(&itm, 0, dateObj, "Planner description 436", REP_YEARLY, exp, -1);

    if (rc != PLANNER_STATUS__OK) {
        printf("...buildItemDummyFunction: Received error: %d\n.", rc);
        exit(EXIT_FAILURE);
    }

    return itm;
}

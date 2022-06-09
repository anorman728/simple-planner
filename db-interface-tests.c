#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "date-functions.h"
#include "db-interface.h"
#include "planner-functions.h"

void testSavingMultipleRecords();
void deleteFileIfExists(char *filename);

char *testDb = "./testing.db";

int main()
{
    testSavingMultipleRecords();

    deleteFileIfExists(testDb);
}

void testSavingMultipleRecords()
{
    printf("Starting testSavingMultipleRecords.\n");

    deleteFileIfExists(testDb);

    db_interface_initialize(testDb);

    char *desc = "test entry 9631";

    PlannerItem *testObj = buildItem(
        0,
        buildDate(2022, 6, 5),
        desc,
        rep_N,
        buildDate(0, 1, 1),
        0
    );

    PlannerItem *testObjs[1];

    testObjs[0] = testObj;

    db_interface_save(testObjs, 1);

    long returned_id = testObj->id;

    printf("Inserted object resulting in id #%ld\n", returned_id);

    // Check the results.  (Also tests db_interface_get.)

    PlannerItem *retrieved = db_interface_get(testObj->id);

    if (strcmp(retrieved->desc, desc) != 0) {
        printf("FAILURE: desc is not retrieved correctly.\n");
    }

    // Don't bother with everything else at this point.

    freeItem(retrieved);


    // Test saving an existing item *with* another new item.

    desc = "second test entry 5316";

    PlannerItem *testObj2 = buildItem(
        0,
        buildDate(2012, 7, 15),
        desc,
        rep_W,
        buildDate(2031, 1, 1),
        -1
    );

    // Change the description of first to check that it's being saved.
    char *newstr = "new description 72713616";
    testObj->desc = (char *) realloc(testObj->desc, strlen(newstr) + 1);
    strcpy(testObj->desc, newstr);

    PlannerItem *testObjs2[2];
    testObjs2[0] = testObj;
    testObjs2[1] = testObj2;

    db_interface_save(testObjs2, 2);

    long second_returned_id = testObj2->id;
    printf("Inserted second object resulting in id #%ld\n", second_returned_id);

    // Go ahead and free these-- They're not needed anymore.
    freeItem(testObj);
    freeItem(testObj2);

    // Now test the results.
    PlannerItem *firstRec = db_interface_get(returned_id);
    PlannerItem *secondRec = db_interface_get(second_returned_id);

    if (strcmp(firstRec->desc, newstr) != 0) {
        printf("FAILURE: desc not retrieved correctly on first record when saved second time.\n");
    }

    if (strcmp(secondRec->desc, desc)!= 0) {
        printf("FAILURE: desc not retrieved correctly on second record.\n");
    }

    freeItem(firstRec);
    freeItem(secondRec);

    db_interface_finalize();

    printf("Completed testSavingMultipleRecords.\n");
}


// Helper functions below this line.

void deleteFileIfExists(char *filename)
{
    FILE *file;

    if ((file = fopen(filename, "r"))) {
        fclose(file);
        remove(filename);
    }
}

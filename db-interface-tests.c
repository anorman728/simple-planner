#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "date-functions.h"
#include "db-interface.h"
#include "planner-functions.h"

void testSavingMultipleRecords();
void testGettingRecordsFromRange();
void deleteFileIfExists(char *filename);

char *testDb = "./testing.db";

int main()
{
    testSavingMultipleRecords();
    testGettingRecordsFromRange();

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
        buildDate(22, 6, 5),
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
        buildDate(12, 7, 15),
        desc,
        rep_W,
        buildDate(31, 1, 1),
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

void testGettingRecordsFromRange()
{
    printf("Starting tetGettingMultipleRecords.\n");

    deleteFileIfExists(testDb);
    db_interface_initialize(testDb);

    // Build multiple items to put into db.

    PlannerItem **testArr = malloc(4 * (sizeof *testArr));

    char *inRes = "In results";
    char *notInRes = "Not in results.";

    testArr[0] = buildItem(
        0,
        buildDate(22, 6, 1),
        notInRes,
        rep_N, // Doesn't matter.
        buildDate(0, 1, 1), // Doesn't matter.
        -1 // Doesn't matter
    );
    testArr[1] = buildItem(
        0,
        buildDate(22, 6, 5),
        inRes,
        rep_N, // Doesn't matter.
        buildDate(0, 1, 1), // Doesn't matter.
        -1
    );
    testArr[2] = buildItem(
        0,
        buildDate(22, 6, 10),
        inRes,
        rep_N, // Doesn't matter.
        buildDate(0, 1, 1), // Doesn't matter.
        -1
    );
    testArr[3] = buildItem(
        0,
        buildDate(22, 6, 15),
        notInRes,
        rep_N, // Doesn't matter.
        buildDate(0, 1, 1), // Doesn't matter.
        -1
    );

    db_interface_save(testArr, 4);
    freeAll(testArr, 4);
    // Go ahead and destroy these; we're going to find them in the database.

    PlannerItem **resultArr = db_interface_range(
        buildDate(22,6,5),
        buildDate(22,6,10)
    );

    int i = 0;

    while (resultArr[i] != NULL) {
        if (strcmp(resultArr[i]->desc, inRes) != 0) {
            char *date = toString(resultArr[i]->date);
            printf("Found invalid desc in results! In date %s.\n", date);
            free(date);
        }
        i++;
    }

    freeAll(resultArr, 3);
    // I just happen to know that there are three for this.  Will be removed in
    // future version.

    db_interface_finalize();

    printf("Completed testGettingRecordsFromRange.\n");
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "date-functions.h"
#include "db-interface.h"
#include "planner-functions.h"

void testBuildError();
void testSavingMultipleRecords();
void testGettingRecordsFromRange();
void deleteFileIfExists(char *filename);

char *testDb = "./testing.db";

int main()
{
    testBuildError();
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

    PlannerItem *testObj;

    buildItem(
        &testObj,
        0,
        buildDate(22, 6, 5),
        desc,
        rep_N,
        buildDate(0, 1, 1),
        0
    );

    PlannerItem *testObjs[2];

    testObjs[0] = testObj;
    testObjs[1] = NULL;

    db_interface_save(testObjs);

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

    PlannerItem *testObj2;

    buildItem(
        &testObj2,
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

    PlannerItem *testObjs2[3];
    testObjs2[0] = testObj;
    testObjs2[1] = testObj2;
    testObjs2[2] = NULL;

    db_interface_save(testObjs2);

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
    printf("Starting testGettingMultipleRecords.\n");

    deleteFileIfExists(testDb);
    db_interface_initialize(testDb);

    // Build multiple items to put into db.

    PlannerItem **testArr = malloc(5 * (sizeof *testArr));

    char *inRes = "In results";
    char *notInRes = "Not in results.";

    buildItem(
        &testArr[0],
        0,
        buildDate(22, 6, 1),
        notInRes,
        rep_N, // Doesn't matter.
        buildDate(0, 1, 1), // Doesn't matter.
        -1 // Doesn't matter
    );
    buildItem(
        &testArr[1],
        0,
        buildDate(22, 6, 5),
        inRes,
        rep_N, // Doesn't matter.
        buildDate(0, 1, 1), // Doesn't matter.
        -1
    );
    buildItem(
        &testArr[2],
        0,
        buildDate(22, 6, 10),
        inRes,
        rep_N, // Doesn't matter.
        buildDate(0, 1, 1), // Doesn't matter.
        -1
    );
    buildItem(
        &testArr[3],
        0,
        buildDate(22, 6, 15),
        notInRes,
        rep_N, // Doesn't matter.
        buildDate(0, 1, 1), // Doesn't matter.
        -1
    );
    testArr[4] = NULL;

    db_interface_save(testArr);
    freeAll(testArr);
    // Go ahead and destroy these; we're going to find them in the database.

    PlannerItem **resultArr = db_interface_range(
        buildDate(22,6,5),
        buildDate(22,6,10)
    );

    int i = 0;

    while (resultArr[i] != NULL) {
        if (strcmp(resultArr[i]->desc, inRes) != 0) {
            char *date = toString(resultArr[i]->date);
            printf("FAILURE: Found invalid desc in results! In date %s.\n", date);
            free(date);
        }
        i++;
    }

    freeAll(resultArr);
    // I just happen to know that there are three for this.  Will be removed in
    // future version.

    db_interface_finalize();

    printf("Completed testGettingRecordsFromRange.\n");
}

void testBuildError()
{
    printf("Starting testBuildError.\n");

    deleteFileIfExists(testDb);
    db_interface_initialize(testDb);

    char *str;
    int rc;

    // Test with db-related error.
    _db_interface_create_db_err();
    rc = db_interface_build_err(&str, DB_INTERFACE__DB_ERROR);

    if (rc) {
        printf("Out of memory on first call to db_interface_build_err.\n");
    }

    if (strcmp(str, "Database error: 1. near \"definitely\": syntax error") != 0) {
        printf("FAILURE: String is not set as expected for db-related error.\n");
    }
    free(str);
    str = NULL;

    // Test with interface error.
    rc = db_interface_build_err(&str, DB_INTERFACE__OK);

    if (rc) {
        printf("Out of memory on second call to db_interface_build_err.\n");
    }

    if (strcmp(str, "Interface error: 0.") != 0) {
        printf("FAILURE: String is not set as expected or interface error.\n");
    }

    db_interface_finalize();
    free(str);
    str = NULL;

    printf("Completed testBuildError.\n");
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

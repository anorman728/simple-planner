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
    printf("...Starting testSavingMultipleRecords.\n");

    deleteFileIfExists(testDb);

    char rc;

    rc = db_interface_initialize(testDb);

    if (rc) {
        printf("ERROR: Could not initialize DB.\n");
        return;
    }

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

    db_interface_save(testObj);

    long returned_id = testObj->id;

    printf("...Inserted object resulting in id #%ld\n", returned_id);

    // Check the results.  (Also tests db_interface_get.)

    PlannerItem *retrieved = db_interface_get(testObj->id);

    if (strcmp(retrieved->desc, desc) != 0) {
        printf("FAILURE: desc is not retrieved correctly.\n");
    }

    // Don't bother with everything else at this point.

    freeItem(retrieved);


    // Test saving an existing item.

    // Change the description to check that it's being saved.
    char *newstr = "new description 72713616";
    testObj->desc = (char *) realloc(testObj->desc, strlen(newstr) + 1);
    strcpy(testObj->desc, newstr);

    db_interface_save(testObj);

    // Go ahead and free these-- They're not needed anymore.
    freeItem(testObj);

    // Now test the results.
    PlannerItem *firstRec = db_interface_get(returned_id);

    if (strcmp(firstRec->desc, newstr) != 0) {
        printf("FAILURE: desc not retrieved correctly on first record when saved second time.\n");
    }

    freeItem(firstRec);

    rc = db_interface_finalize();

    if (rc) {
        printf("ERROR: Could not finalize DB.  Found %d.\n", rc);
        return;
    }

    printf("...Completed testSavingMultipleRecords.\n");
}

void testGettingRecordsFromRange()
{
    printf("...Starting testGettingMultipleRecords.\n");

    char rc;

    deleteFileIfExists(testDb);
    rc = db_interface_initialize(testDb);

    if (rc) {
        printf("ERROR: Could not initialize db. %d\n", rc);
    }

    // Build multiple items to put into db.

    char *inRes = "In results";
    char *notInRes = "Not in results.";

    PlannerItem *testObj;

    buildItem(
        &testObj,
        0,
        buildDate(22, 6, 1),
        notInRes,
        rep_N, // Doesn't matter.
        buildDate(0, 1, 1), // Doesn't matter.
        -1 // Doesn't matter
    );
    db_interface_save(testObj);
    freeItem(testObj);

    buildItem(
        &testObj,
        0,
        buildDate(22, 6, 5),
        inRes,
        rep_N, // Doesn't matter.
        buildDate(0, 1, 1), // Doesn't matter.
        -1
    );
    db_interface_save(testObj);
    freeItem(testObj);

    buildItem(
        &testObj,
        0,
        buildDate(22, 6, 10),
        inRes,
        rep_N, // Doesn't matter.
        buildDate(0, 1, 1), // Doesn't matter.
        -1
    );
    db_interface_save(testObj);
    freeItem(testObj);

    buildItem(
        &testObj,
        0,
        buildDate(22, 6, 15),
        notInRes,
        rep_N, // Doesn't matter.
        buildDate(0, 1, 1), // Doesn't matter.
        -1
    );
    db_interface_save(testObj);
    freeItem(testObj);

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

    rc = db_interface_finalize();

    if (rc) {
        printf("ERROR:  Could not finalize db. %d\n", rc);
    }

    printf("...Completed testGettingRecordsFromRange.\n");
}

void testBuildError()
{
    printf("...Starting testBuildError.\n");

    deleteFileIfExists(testDb);

    char rc;

    rc = db_interface_initialize(testDb);

    if (rc) {
        printf("ERROR: Could not initialize DB.  Found %d.\n", rc);
        return;
    }

    char *str;

    // Test with db-related error.
    _db_interface_create_db_err();
    rc = db_interface_build_err(&str, DB_INTERFACE__DB_ERROR);

    if (rc) {
        printf("ERROR: Out of memory on first call to db_interface_build_err.\n");
        return;
    }

    if (strcmp(str, "Database error: 1. near \"definitely\": syntax error") != 0) {
        printf("FAILURE: String is not set as expected for db-related error.\n");
    }
    free(str);
    str = NULL;

    // Test with interface error.
    rc = db_interface_build_err(&str, DB_INTERFACE__OK);

    if (rc) {
        printf("ERROR: Out of memory on second call to db_interface_build_err.\n");
        return;
    }

    if (strcmp(str, "Interface error: 0.") != 0) {
        printf("FAILURE: String is not set as expected or interface error.\n");
    }

    free(str);
    str = NULL;

    rc = db_interface_finalize();

    if (rc) {
        printf("ERROR: Could not finalize DB.  Found %d.\n", rc);
    }

    printf("...Completed testBuildError.\n");
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

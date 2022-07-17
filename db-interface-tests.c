#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "date-functions.h"
#include "db-interface.h"
#include "planner-functions.h"

void testBuildError();
void testSaving();
void testGettingRecordsFromRange();
void testGettingNonexistentRecord();
void testUpdatingDesc();
void testDelete();

void deleteFileIfExists(char *filename);
void printError(char *desc, char rc);

char *testDb = "./testing.db";

int main()
{
    testBuildError();
    testSaving();
    testGettingRecordsFromRange();
    testGettingNonexistentRecord();
    testUpdatingDesc();
    testDelete();

    deleteFileIfExists(testDb);
}

void testBuildError()
{
    printf("...Starting testBuildError.\n");

    deleteFileIfExists(testDb);

    char rc;

    rc = db_interface_initialize(testDb);

    if (rc) {
        printError("Could not initialize DB.", rc);
        return;
    }

    char *str;

    // Test with db-related error.
    _db_interface_create_db_err();
    rc = db_interface_build_err(&str, DB_INTERFACE__DB_ERROR);

    if (rc) {
        printError("Out of memory on first call to db_interface_build_err", rc);
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
        printError("Out of memory on second call to db_interface_build_err.", rc);
        return;
    }

    if (strcmp(str, "No error for db interface.") != 0) {
        printf("FAILURE: String is not set as expected for interface error.\n");
    }

    free(str);
    str = NULL;

    rc = db_interface_finalize();

    if (rc) {
        printError("Could not finalize DB.", rc);
    }

    printf("...Completed testBuildError.\n");
}

void testSaving()
{
    printf("...Starting testSaving.\n");

    deleteFileIfExists(testDb);

    char rc;

    if ((rc = db_interface_initialize(testDb))) {
        printError("during initialization", rc);
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

    if ((rc = db_interface_save(testObj))) {
        printError("during first save", rc);
        return;
    }

    long returned_id = testObj->id;

    printf("...Inserted object resulting in id #%ld\n", returned_id);

    // Check the results.  (Also tests db_interface_get.)

    PlannerItem *retrieved;

    if ((rc = db_interface_get(&retrieved, testObj->id))) {
        printError("during first retrieval",rc);
        return;
    }

    if (strcmp(retrieved->desc, desc) != 0) {
        printf("FAILURE: desc is not retrieved correctly.\n");
    }

    // Don't bother with everything else at this point.

    freeItem(retrieved);


    // Test saving an existing item.

    // Change the description to check that it's being saved.
    char *newstr = "new description 72713616";
    testObj->desc = (char *) realloc(testObj->desc, strlen(newstr) + 1);

    if (testObj->desc == NULL) {
        printf("Out of memory during second save.\n");
        return;
    }

    strcpy(testObj->desc, newstr);

    if ((rc = db_interface_save(testObj))) {
        printError("during second save", rc);
        return;
    }
    freeItem(testObj);

    // Now test the results.
    PlannerItem *firstRec;

    if ((rc = db_interface_get(&firstRec, returned_id))) {
        printError("during second retrieval", rc);
        return;
    }

    if (strcmp(firstRec->desc, newstr) != 0) {
        printf("FAILURE: desc not retrieved correctly on first record when saved second time.\n");
    }

    freeItem(firstRec);

    if ((rc = db_interface_finalize())) {
        printError("during finalization", rc);
        return;
    }

    printf("...Completed testSaving.\n");
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
    if ((rc = db_interface_save(testObj))) {
        printError("first save", rc);
    }
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
    if ((rc = db_interface_save(testObj))) {
        printError("second save", rc);
    }
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
    if ((rc = db_interface_save(testObj))) {
        printError("third save", rc);
    }
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
    if ((rc = db_interface_save(testObj))) {
        printError("fourth save", rc);
    }
    freeItem(testObj);

    PlannerItem *result;

    while ((rc = db_interface_range(
        &result,
        buildDate(22,6,5),
        buildDate(22,6,10)
    )) == DB_INTERFACE__CONT) {
        if (strcmp(result->desc, inRes) != 0) {
            char *date;
            toString(&date, result->date);
            printf("FAILURE: Found invalid desc in results! In date %s.\n", date);
            free(date);
        }
        freeItem(result);
    }

    if (rc) {
        printError("final call to db_interface_rage", rc);
    }

    if ((rc = db_interface_finalize())) {
        printError("Could not finalize db.", rc);
    }

    printf("...Completed testGettingRecordsFromRange.\n");
}

void testGettingNonexistentRecord()
{
    printf("...Starting testGettingNonexistentRecord.\n");

    deleteFileIfExists(testDb);

    char rc;

    if ((rc = db_interface_initialize(testDb))) {
        printError("during initialization", rc);
        return;
    }

    PlannerItem *testObj;

    buildItem(
        &testObj,
        0,
        buildDate(22,6,5),
        "some description",
        rep_N,
        buildDate(0, 1, 1),
        0
    );

    if ((rc = db_interface_save(testObj))) {
        printError("during save", rc);
        return;
    }

    long returned_id = testObj->id;

    printf("...Inserted object resulting in id #%ld\n", returned_id);

    freeItem(testObj);
    testObj = NULL;

    PlannerItem *RetItm;

    printf("...Retrieving existing item.\n");

    if ((rc = db_interface_get(&RetItm, returned_id))) {
        printError("on getting existing item", rc);
        return;
    }

    if (RetItm == NULL) {
        printf("FAILURE:  Existing item is returned as null.\n");
        return;
    }

    freeItem(RetItm);

    printf("...Retrieving nonexisting item.\n");

    if ((rc = db_interface_get(&RetItm, returned_id + 1))) {
        printError("on getting nonexisting item", rc);
        return;
    }

    if (RetItm != NULL) {
        printf("FAILURE: Retrieved item is not null.\n");
        return;
    }

    if ((rc = db_interface_finalize())) {
        printError("during finalization", rc);
        return;
    }

    printf("...Completed testGettingNonexistentRecord.\n");
}

void testUpdatingDesc()
{
    printf("...Starting testUpdatingDesc.\n");

    char rc;

    deleteFileIfExists(testDb);
    if ((rc = db_interface_initialize(testDb))) {
        printf("ERROR: Could not initialize db.  %d\n", rc);
        return;
    }

    // Saving item to be modified.
    PlannerItem *testObj;

    buildItem(
        &testObj,
        0,
        buildDate(22,6,1),
        "old description",
        rep_N, // Doesn't matter.
        buildDate(0, 1, 1), // Doesn't matter.
        -1 // Doesn't matter
    );
    if ((rc = db_interface_save(testObj))) {
        printError("saving", rc);
        return;
    }

    long id = testObj->id;

    freeItem(testObj);

    // Modifying object.
    char *newdesc = "new description purple hippopatamus";
    rc = db_interface_update_desc(id, newdesc);

    if (rc) {
        printf("ERROR: %d\n", rc);
    }

    // Getting object to test.
    PlannerItem *retrieved;

    if ((rc = db_interface_get(&retrieved, id))) {
        printError("retrieving saved item", rc);
    }

    if (strcmp(retrieved->desc, newdesc) != 0) {
        printf("FAILURE: description was not saved as expected.\n");
    }

    freeItem(retrieved);

    if ((rc = db_interface_finalize())) {
        printError("during finalization", rc);
        return;
    }

    printf("...Completed testUpdatingDesc.\n");
}

void testDelete()
{
    printf("...Starting testDelete.\n");

    char rc;

    deleteFileIfExists(testDb);
    if ((rc = db_interface_initialize(testDb))) {
        printf("ERROR: Could not initialize db.  %d\n", rc);
    }

    // Saving item to be deleted.
    PlannerItem *testObj;

    buildItem(
        &testObj,
        0,
        buildDate(22,6,1),
        "to be deleted",
        rep_N, // Doesn't matter.
        buildDate(0, 1, 1), // Doesn't matter
        -1 // Doesn't matter.
    );

    printf("...Saving object to be deleted.\n");

    if ((rc = db_interface_save(testObj))) {
        printError("saving", rc);
    }

    long id = testObj->id;
    freeItem(testObj);

    // Deleting object.
    printf("...Deleting object.\n");
    rc = db_interface_delete(id);

    if (rc) {
        printError("when deleting", rc);
    }

    PlannerItem *returnObj;

    if ((rc = db_interface_get(&returnObj, id))) {
        printError("when getting nonexisting object", rc);
        return;
    }

    if (returnObj != NULL) {
        printf("FAILURE: Nonnull object returned get db_interface_get.\n");
        return;
    }

    if ((rc = db_interface_finalize())) {
        printError("during finalization", rc);
        return;
    }

    printf("...Completed testDelete.\n");
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

void printError(char *desc, char rc) {
    char *str;
    db_interface_build_err(&str, rc);
    printf("ERROR %s: %s\n", desc, str);
    free(str);
}

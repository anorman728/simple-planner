#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "date-functions.h"
#include "db-interface.h"
#include "planner-functions.h"

void testBuildError();
void testSaving();
void testGettingRecordsFromRange();
void testGettingRecordsForDay();
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
    testGettingRecordsForDay();
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
        REP_NONE
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
        REP_NONE // Doesn't matter.
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
        REP_NONE // Doesn't matter.
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
        REP_NONE // Doesn't matter.
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
        REP_NONE
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

void testGettingRecordsForDay()
{
    printf("...Starting testGettingRecordsForDay.\n");

    char rc;
    int count;

    deleteFileIfExists(testDb);
    rc = db_interface_initialize(testDb);

    if (rc) {
        printf("ERROR: Could not initialize db. %d\n", rc);
    }

    PlannerItem *testObj;
    PlannerItem *result = NULL;

    buildItem(
        &testObj,
        0,
        buildDate(22, 6, 1),
        "Test object.",
        REP_NONE
    );
    if ((rc = db_interface_save(testObj))) {
        printError("first save", rc);
    }
    freeItem(testObj);

    // Nothing returned;

    if ((rc = db_interface_day(&result, buildDate(22,6,2))) != DB_INTERFACE__OK) {
        printf("FAILURE:  Found something when should be nothing.");
    }
    if (result != NULL) {
        printf("FAILURE: result should be null.\n");
    }

    // One no repetition returned.

    count = 0;
    while ((rc = db_interface_day(&result, buildDate(22,6,1)))) {
        count++;
        if (result->rep != REP_NONE) {
            printf("FAILURE: On no repetition returned: found repetition that should not exist.\n");
        }
        if (strcmp(result->desc, "Test object.") != 0) {
            printf("FAILURE: On no repetition returned: string does not match.\n");
        }
        freeItem(result);
    }

    if (count != 1) {
        printf("FAILURE: Should have found exactly one with no repetition returned.\n");
    }

    // One yearly repetition returned;

    buildItem(
        &testObj,
        0,
        buildDate(22,11,4),
        "Yearly rep.",
        REP_YEARLY
    );
    if ((rc = db_interface_save(testObj))) {
        printError("Second save", rc);
    }
    freeItem(testObj);

    count = 0;
    while ((rc = db_interface_day(&result, buildDate(22,11,4)))) {
        count++;
        if (result->rep != REP_YEARLY) {
            printf("FAILURE: On yearly returned, found no wrong kind of rep.\n");
        }
        if (strcmp(result->desc, "Yearly rep.") != 0) {
            printf("FAILURE: On yearly returned, found wrong description.");
        }
        freeItem(result);
    }

    if (count != 1) {
        printf("FAILURE: SHould have found exactly one with yearly repetition.\n");
    }

    // Both yearly and no rep.

    buildItem(
        &testObj,
        0,
        buildDate(22,10,4),
        "Both",
        REP_NONE
    );
    if ((rc = db_interface_save(testObj))) {
        printError("Third save", rc);
    }
    long int id01 = testObj->id;
    freeItem(testObj);

    buildItem(
        &testObj,
        0,
        buildDate(22,10,4),
        "Both",
        REP_YEARLY
    );
    if ((rc = db_interface_save(testObj))) {
        printError("Fourth save", rc);
    }
    long int id02 = testObj->id;
    freeItem(testObj);

    count = 0;

    while ((rc = db_interface_day(&result, buildDate(22,10,4)))) {
        count++;
        if (result->id == id01 && result->rep != REP_NONE) {
            printf("FAILURE: Wrong kind of repetition returned for first item.");
        }
        if (result->id == id02 && result->rep != REP_YEARLY) {
            printf("FAILURE: Wrong kind of repetition returned for second item.");
        }
        if (strcmp(result->desc, "Both") != 0) {
            printf("FAILURE: Wrong description returned for testing both.");
        }
        freeItem(result);
    }

    if (count != 2) {
        printf("FAILURE: Did not find enough results returned.");
    }

    // End tests.

    if ((rc = db_interface_finalize())) {
        printError("Could not finalize db.", rc);
    }

    printf("...Completed testGettingRecordsForDay.\n");
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
        REP_NONE
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
        REP_NONE
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
        REP_NONE
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

    // Now check that it still exists, so it was only soft-deleted.

    sqlite3_stmt *stmt = NULL;
    char *sqldum = "SELECT desc,del FROM items WHERE id = ?;";
    rc = sqlite3_prepare_v2(db_interface_get_db(), sqldum, -1, &stmt, 0);
    rc = sqlite3_bind_int(stmt, 1, id);
    rc = sqlite3_step(stmt);

    char *resdesc = (char *) malloc(strlen((const char*) sqlite3_column_text(stmt, 0)) + 1);
    strcpy(resdesc, (const char*) sqlite3_column_text(stmt, 0));
    if (strcmp(resdesc, "to be deleted") != 0) {
        printf("FAILURE: Did not find expected item that should only be soft-deleted.");
    }

    char isdel = sqlite3_column_int(stmt, 1);
    if (isdel != 1) {
        printf("FAILURE: Item found, but was not soft-deleted.");
    }

    sqlite3_finalize(stmt);
    free(resdesc);
    resdesc = NULL;

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

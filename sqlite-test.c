#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>

// This is just a program to mess around with SQLite, figuring out how to use
// it.

// https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm
// https://www.sqlite.org/cintro.html (Pretty bad.  No examples.  But good when combined with below.)
// https://github.com/robchou/sqlite3-c-example/blob/master/example_sqlite_stmt.c

/** @var char* Name for db file. */
char *dbFileName = "planner.db";

/** @var char* Data returned from db. */
char *dataFromDb;

static int callbackFunc(void *data, int argc, char **argv, char **colname);

int main(int argc, char *argv[])
{
    // An IRL example would, of course, break this into multiple parts.
    // An IRL example would also handle freeing memory better when getting
    // errors *and* would return (since there are no exceptions to propagate in
    // C!)

    char *dbFile = (char *) malloc(100);
    if (argc > 1) {
        strcpy(dbFile, argv[1]);
        strcat(dbFile, "/");
    } else {
        strcpy(dbFile, "./");
    }
    strcat(dbFile, dbFileName);

    sqlite3 *db;
    int rc;
    char *zErrMsg;

    rc = sqlite3_open(dbFile, &db);
    free(dbFile);

    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    // Create table.
    char *createTable = "CREATE TABLE testtable( \
        id INTEGER PRIMARY KEY AUTOINCREMENT, \
        someval TEXT NOT NULL, \
        someint INTEGER NOT NULL, \
        somenum REAL NOT NULL \
    );";

    rc = sqlite3_exec(db, createTable, 0, 0, &zErrMsg);
    if (rc) {
        printf("Error: %s\n", zErrMsg);
    }
    sqlite3_free(zErrMsg);

    // Insert two rows.  USES PREPARED STATEMENTS.
    char *insertRow = "INSERT INTO testtable(someval, someint, somenum) \
        VALUES (?,?,?);";

    char *someval;
    int someint;
    double somenum;

    sqlite3_stmt *stmt;

    rc = sqlite3_prepare_v2(
        db,
        insertRow,
        -1, // Read sql string up to null terminator.
        &stmt, // stmt is already a pointer, so this is a pointer to a pointer, I guess?
        0 // Pointer to unused portion of sql (only used if third arg is nonnegative).
    );

    if (rc) {
        printf("Error: %s\n", sqlite3_errmsg(db));
    }

    someval = "test value 925";
    someint = 7;
    somenum = 1.234;

    rc = sqlite3_bind_text(
        stmt,
        1, // the first "?" in the query string
        someval,
        -1, // Similar to before, read `someval` up to null terminator.
        0 // The description for this wasn't worth reading.  Following the example of the github sample.
    );
    // Then do usual testing with RC.  No string passed to bind, so need to use sqlite3_errmsg(db).

    rc = sqlite3_bind_int(stmt, 2, someint);
    rc = sqlite3_bind_double(stmt, 3, somenum);

    // Execute statement.
    rc = sqlite3_step(stmt);
    // Should test now if rc equals SQLITE_DONE.
    rc = sqlite3_reset(stmt);

    // Not doing blob example rn.

    // Do it all again with the second record.
    someval = "test value 23535";
    someint = 9;
    somenum = 5.678;

    sqlite3_bind_text(stmt, 1, someval, -1, 0);
    sqlite3_bind_int(stmt, 2, someint);
    sqlite3_bind_double(stmt, 3, somenum);

    sqlite3_step(stmt);
    rc = sqlite3_reset(stmt);

    // Third time, this time with a non-unique someint.
    someval = "test value 3447";
    someint = 7;
    somenum = 9.012;

    sqlite3_bind_text(stmt, 1, someval, -1, 0);
    sqlite3_bind_int(stmt, 2, someint);
    sqlite3_bind_double(stmt, 3, somenum);

    sqlite3_step(stmt);
    //rc = sqlite3_reset(stmt); // Not necessary because...

    rc = sqlite3_finalize(stmt); // ...this frees the prepared statement.
    // Should return SQLITE_OK.  https://www.sqlite.org/c3ref/finalize.html


    // Retrieving data from the db with prepared statements.
    // This example skips virtually all error checking.
    char *sqldum;

    sqldum = "SELECT someval FROM testtable WHERE someint = ?;";
    sqlite3_prepare_v2(db, sqldum, -1, &stmt, 0);
    sqlite3_bind_int(stmt, 1, 7);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("Found value: %s\n", sqlite3_column_text(stmt, 0));
        // This one starts at zero even though the prepared statements starts at
        // one.  No idea why.
        // There's also other data types besides text-- https://www.sqlite.org/c3ref/column_blob.html

        // The value returned by sqlite3_column_text is const *char, so will
        // probably need to cast it to (char *) to assign it.
    }
    sqlite3_finalize(stmt);
    printf("Done testing prepared statement.\n");


    // Retrieving data from the db without prepared statements.
    // Note: Using prepared statements is way better!  Even apart from being
    // more secure, it doesn't require callback functions.
    sqldum = "SELECT someval FROM testtable;";

    char *data = "This will be freed when the callbackFunc is finished being run.";

    dataFromDb = (char *) malloc(1);
    strcpy(dataFromDb, ""); // Empty string for now.  Don't know how long it will be.

    rc = sqlite3_exec(db, sqldum, callbackFunc, data, &zErrMsg);

    if (rc == SQLITE_OK) {
        printf("Done running non-prepared SELECT query.\n");
        printf("Result: %s\n", dataFromDb); // Should print "Result: testvalue925test value 23535"
    } else {
        fprintf(stderr, "Error running query: %s\n", zErrMsg);
    }

    free(dataFromDb);
    sqlite3_free(zErrMsg);
    sqlite3_close(db);

    return 0;
}

static int callbackFunc(void *data, int argc, char **argv, char **colname)
{
    // This function is run for every record returned.

    // After the last time it's run, data is cleared.  I have no idea why.
    // But it makes me think the point of the value is to pass data into this
    // function, not to get data out of this function.

    // The point of dataFromDb is to get data back into the main method.  I
    // would want better code organization for this for real-world application.
    // (Maybe have the function name and the data variable name related?  And
    // declared at the same place.)
    // In this case, dataFromDb is just all of the data from "someval" column
    // concatenated into one string.

    dataFromDb = (char *) realloc(dataFromDb, strlen(argv[0]) + strlen(dataFromDb) + 1);
    strcat(dataFromDb, argv[0]);

    return 0;
    // Don't forget this!  It will only run for first record then say "query
    // aborted" if it's not there.
}

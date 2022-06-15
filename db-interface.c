#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "date-functions.h"
#include "planner-functions.h"

/** var dbFile Pointer to sqlite3 database object. */
static sqlite3 *dbFile;

static void saveNew(PlannerItem *item);
static void saveExisting(PlannerItem *item);
static PlannerItem **getFromWhere(char *where, int *values, int count);

static void updateDatabase();
static int doesDatabaseExist();
static void createDbV1();

/**
 * Open the database file.  Create one if it doesn't exist.
 *
 * @param   filename    String representing filename path.
 */
void db_interface_initialize(char *filename)
{
    // TODO: Return int with status.
    int rc = sqlite3_open(filename, &dbFile);

    if (rc) {
        // TODO: Return error status here.
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(dbFile));
    }

    updateDatabase();
}

/**
 * Close the database file.
 */
void db_interface_finalize()
{
    sqlite3_close(dbFile);
}

/**
 * Save amt of the items.
 *
 * TODO: To be consistent with the range method, make last object null pointer.
 *
 * @param   items   Array of items to save.
 * @param   amt     Number of items to save.
 */
void db_interface_save(PlannerItem *items[], int amt)
{
    for (int i = 0; i < amt; i++) {
        if (items[i]->id == 0) {
            saveNew(items[i]);
        } else {
            saveExisting(items[i]);
        }
    }
}

/**
 * Retrieve single PlannerItem object.
 *
 * @param   id  Id of row to retrieve.
 */
PlannerItem *db_interface_get(int id)
{
    int vals[1];
    vals[0] = id;

    PlannerItem **dumval = getFromWhere("id = ?", vals, 1);

    PlannerItem *returnVal = dumval[0];

    //free(dumval[1]);
    // This is a null pointer.  I'm not 100% sure freeing it is necessary,
    // because neither having it here nor not having it will generate an error
    // or warning.
    // Update: I read online that this does nothing.

    free(dumval); // Need to free the array itself, since it's on the heap.

    return returnVal;
}

/**
 * Retrieve array of PlannerItem pointers from date range.
 *
 * Last object is null pointer, signifying end of array.
 *
 * @param   lower   Lower bound (inclusive)
 * @param   upper   Upper bound (inclusive)
 */
PlannerItem **db_interface_range(Date lower, Date upper)
{
    int vals[2];
    vals[0] = toInt(upper);
    vals[1] = toInt(lower);

    return getFromWhere("date <= ? AND date >= ?", vals, 2);
}


// Static functions below this line.

/**
 * Save a new record to the database, and save the id.
 *
 * @param   item    A PlannerItem pointer.
 */
static void saveNew(PlannerItem *item)
{
    char *insertRow = "INSERT INTO items(date, desc, rep, exp, done) \
        VALUES (?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt;

    sqlite3_prepare_v2(dbFile, insertRow, -1, &stmt, 0);

    sqlite3_bind_int(stmt, 1, toInt(item->date));
    sqlite3_bind_text(stmt, 2, item->desc, -1, 0);
    sqlite3_bind_int(stmt, 3, item->rep);
    sqlite3_bind_int(stmt, 4, toInt(item->exp));
    sqlite3_bind_int(stmt, 5, item->done);

    sqlite3_step(stmt);

    long iddum;
    iddum = sqlite3_last_insert_rowid(dbFile);

    item->id = iddum;

    sqlite3_finalize(stmt);
}

/**
 * Save an existing record to the database.
 *
 * @param   item    A PlannerItem pointer.
 */
static void saveExisting(PlannerItem *item)
{
    char *updateRow = "UPDATE items SET date = ?, desc = ?, rep = ?, exp = ?,"
     " done = ? WHERE id = ?;";

    sqlite3_stmt *stmt;

    sqlite3_prepare_v2(dbFile, updateRow, -1, &stmt, 0);

    sqlite3_bind_int(stmt,1, toInt(item->date));
    sqlite3_bind_text(stmt,2, item->desc, -1, 0);
    sqlite3_bind_int(stmt,3, item->rep);
    sqlite3_bind_int(stmt,4, toInt(item->exp));
    sqlite3_bind_int(stmt,5, item->done);
    sqlite3_bind_int(stmt,6, item->id);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

/**
 * Get an array of PlannerItem pointers from where clause and the integers to be
 * bound to it.  (This assumes only integers will be bound.)  Resulting array is
 * on heap and needs to be freed.
 *
 * Last object in returned array is null pointer to signify end of list.
 *
 * Helper method for db_interface_get and db_interface_range.
 *
 * @param   where   WHERE clause, including ?s.
 * @param   values  Array of integers to bind.
 * @param   count   Number of integers to bind (i.e., count of "values").
 */
static PlannerItem **getFromWhere(char *where, int *values, int count)
{
    // Build SQL.

    char *sqldum = "SELECT id,date,desc,rep,exp,done FROM items WHERE ";
    // Not using "SELECT *" because the columns are only identified by number,
    // so explicitly naming the columns makes it more future-proof and easier to
    // update.

    char sql[strlen(sqldum) + strlen(where) + 2]; // Semicol and null term.
    strcpy(sql, sqldum);
    strcat(sql, where);
    strcat(sql, ";");

    // Make prepared statement and bind values.

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(dbFile, sql, -1, &stmt, 0);

    for (int i = 0; i < count; i++) {
        sqlite3_bind_int(stmt, i + 1, values[i]);
    }

    int final = 0;
    PlannerItem **returnVal = malloc(sizeof *returnVal);
    // Will be at least one, for the null at the end.

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        returnVal = realloc(returnVal, (++final + 1) * (sizeof *returnVal));

        returnVal[final - 1] = buildItem(
            sqlite3_column_int(stmt, 0),
            toDate(sqlite3_column_int(stmt, 1)),
            (char *) sqlite3_column_text(stmt, 2),
            sqlite3_column_int(stmt, 3),
            toDate(sqlite3_column_int(stmt, 4)),
            sqlite3_column_int(stmt, 5)
        );
    }
    returnVal[final] = NULL;

    sqlite3_finalize(stmt);

    return returnVal;
}


// Database update functions.

/**
 * Update database to latest version.
 */
static void updateDatabase()
{
    if (!doesDatabaseExist()) {
        createDbV1();
    }

    // When need newer database versions, will want some kind of "get database
    // version" function with cascading updates based on the version number.
    // The "value" in the database's meta table is text, though, so will need to
    // use one of those functions to convert c strings to int.  (Remember, can't
    // just do it through casting.)
}

/**
 * Check if there is a database to update.
 */
static int doesDatabaseExist()
{
    sqlite3_stmt *stmt;

    char *sqldum = "SELECT count(*) as count FROM sqlite_master \
    WHERE type='table' AND name=?";

    sqlite3_prepare_v2(dbFile, sqldum, -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, "meta", -1, 0);

    sqlite3_step(stmt);

    int val = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return val;
}

/**
 * Create version 1 of database.
 */
static void createDbV1()
{
    char *sqldum;
    int rc;
    char *zErrMsg;

    // Create meta table.
    sqldum = "CREATE TABLE meta("
        " name TEXT NOT NULL,"
        " desc TEXT NOT NULL,"
        " value TEXT NOT NULL"
    ");";

    rc = sqlite3_exec(dbFile, sqldum, 0, 0, &zErrMsg);
    if (rc) {
        fprintf(stderr, "Error while creating db: %s\n", zErrMsg);
    }
    sqlite3_free(zErrMsg);

    // Put version into meta table.
    sqldum = "INSERT INTO meta("
        " name,"
        " desc,"
        " value"
    ") VALUES("
        " \"version\","
        " \"The version number.\","
        " \"1\""
    ");";

    rc = sqlite3_exec(dbFile, sqldum, 0, 0, &zErrMsg);
    if (rc) {
        fprintf(stderr, "Error while inserting first record into \"meta\" table: %s\n", zErrMsg);
    }
    sqlite3_free(zErrMsg);

    // Create planner_items table.
    sqldum = "CREATE TABLE items("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " date INTEGER NOT NULL,"
        " desc TEXT,"
        " rep INTEGER NOT NULL,"
        " exp INTEGER NOT NULL,"
        " done INTEGER NOT NULL"
    ");";
    rc = sqlite3_exec(dbFile, sqldum, 0, 0, &zErrMsg);
    if (rc) {
        fprintf(stderr, "Error while creating \"items\" table: %s\n.", zErrMsg);
    }
    sqlite3_free(zErrMsg);
}

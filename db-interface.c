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
static void db_interface_build_err__db(char **str);
static void db_interface_build_err__ifce(char **str, int code);

static void updateDatabase();
static int doesDatabaseExist();
static void createDbV1();

/** This is the error code from SQLite. */
int dbErrCode = 0;

// Constants

const char DB_INTERFACE__OK = 0;

const char DB_INTERFACE__DB_ERROR = 1;


/**
 * Open the database file.  Create one if it doesn't exist.
 *
 * @param   filename    String representing filename path.
 */
void db_interface_initialize(char *filename)
{
    // TODO: Return int with status.
    // https://sqlite.org/c3ref/open.html
    dbErrCode = sqlite3_open(filename, &dbFile);

    if (dbErrCode) {
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
    // https://sqlite.org/c3ref/close.html
    dbErrCode = sqlite3_close(dbFile);
}

/**
 * Save array of PlannerItem objects.
 *
 * @param   items   Array of items to save.
 */
void db_interface_save(PlannerItem *items[])
{
    int i = 0;

    while (items[i] != NULL) {
        if (items[i]->id == 0) {
            saveNew(items[i]);
        } else {
            saveExisting(items[i]);
        }
        i++;
    }
}

/**
 * Get the most recent error code from SQLite.
 */
int db_interface_get_db_err()
{
    return dbErrCode;
}

/**
 * Build error string from return code, for printing.
 *
 * If the error code is for SQLite, this will use the most recent error.
 *
 * The resulting string is on heap memory and must be freed!
 *
 * @param   str     HEAP.  Pointer to string.
 * @param   code    Previously-returned error code.
 */
void db_interface_build_err(char **str, int code)
{
    // I decided to use heap memory for this because it makes the code
    // significantly easier and passing a function-scope pointer to this
    // function is not memory-safe and runs the risk of truncating the output
    // string.  There is also some precedent here, because this is also how
    // sqlite3_exec handles their errmsg string, if it's not null.  Having to
    // free the memory is worth it to have manageable code.  Can look at the
    // previous commit to see what it looked like before.

    *str = malloc(1);
    strcpy(*str, "");

    if (code == DB_INTERFACE__DB_ERROR) {
        db_interface_build_err__db(str);
        return;
    }

    db_interface_build_err__ifce(str, code);

}

/**
 * Intentionally create a database error, for purposes of testing.
 */
void _db_interface_create_db_err()
{
    char *sqldum = "definitely not valid sql code";

    dbErrCode = sqlite3_exec(dbFile, sqldum, 0, 0, NULL);
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

    free(dumval); // Need to free the array itself, since it's on the heap.
    // Don't need to free second object.  It's a null pointer.

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

    dbErrCode = sqlite3_prepare_v2(dbFile, insertRow, -1, &stmt, 0);

    dbErrCode = sqlite3_bind_int(stmt, 1, toInt(item->date));
    dbErrCode = sqlite3_bind_text(stmt, 2, item->desc, -1, 0);
    dbErrCode = sqlite3_bind_int(stmt, 3, item->rep);
    dbErrCode = sqlite3_bind_int(stmt, 4, toInt(item->exp));
    dbErrCode = sqlite3_bind_int(stmt, 5, item->done);

    if ((dbErrCode = sqlite3_step(stmt)) != SQLITE_ROW) {
        // TODO: Return here.
    }

    long iddum;
    iddum = sqlite3_last_insert_rowid(dbFile);

    item->id = iddum;

    dbErrCode = sqlite3_finalize(stmt);
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

    dbErrCode = sqlite3_prepare_v2(dbFile, updateRow, -1, &stmt, 0);

    dbErrCode = sqlite3_bind_int(stmt,1, toInt(item->date));
    dbErrCode = sqlite3_bind_text(stmt,2, item->desc, -1, 0);
    dbErrCode = sqlite3_bind_int(stmt,3, item->rep);
    dbErrCode = sqlite3_bind_int(stmt,4, toInt(item->exp));
    dbErrCode = sqlite3_bind_int(stmt,5, item->done);
    dbErrCode = sqlite3_bind_int(stmt,6, item->id);

    if ((dbErrCode = sqlite3_step(stmt)) != SQLITE_ROW) {
        // TODO: Return here.
    }

    dbErrCode = sqlite3_finalize(stmt);
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
    // https://sqlite.org/c3ref/prepare.html
    dbErrCode = sqlite3_prepare_v2(dbFile, sql, -1, &stmt, 0);

    for (int i = 0; i < count; i++) {
        // https://sqlite.org/c3ref/bind_blob.html
        dbErrCode = sqlite3_bind_int(stmt, i + 1, values[i]);
    }

    int final = 0;
    int rc;
    PlannerItem **returnVal = malloc(sizeof *returnVal);
    // Will be at least one, for the null at the end.

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        returnVal = realloc(returnVal, (++final + 1) * (sizeof *returnVal));

        rc = buildItem(
            &returnVal[final - 1],
            sqlite3_column_int(stmt, 0),
            toDate(sqlite3_column_int(stmt, 1)),
            (char *) sqlite3_column_text(stmt, 2),
            sqlite3_column_int(stmt, 3),
            toDate(sqlite3_column_int(stmt, 4)),
            sqlite3_column_int(stmt, 5)
        );

        if (rc != PLANNER_STATUS__OK) {
            printf("db-interface.getFromWhere: Received error %d.  Exiting.\n.", rc);
            exit(EXIT_FAILURE);
        }
    }
    returnVal[final] = NULL;

    dbErrCode = sqlite3_finalize(stmt);

    return returnVal;
}

/**
 * Set error string from database error.
 *
 * @param   str     HEAP.  Pointer to string.
 * @param   code
 */
static void db_interface_build_err__db(char **str)
{
    char *fmt = "Database error: %d. ";
    int strlen01 = snprintf(NULL, 0, fmt, dbErrCode);

    char *dbMsg = (char *) sqlite3_errmsg(dbFile);

    int sizedum = strlen01 + strlen(dbMsg) + 1;
    *str = realloc(*str, sizedum);
    snprintf(*str, sizedum, fmt, dbErrCode);
    strcat(*str, dbMsg);
}

/**
 * Set error string for interface error.
 *
 * @param   str     HEAP.  Pointer to string.
 * @param   code
 */
static void db_interface_build_err__ifce(char **str, int code)
{
    char *fmt = "Interface error: %d.";
    // TODO: May want to add strings to this, but not important enough at the
    // moment.  Right now, there are no interface errors anyway.
    int strlen01 = snprintf(NULL, 0, fmt, code);

    *str = realloc(*str, strlen01 + 1);
    snprintf(*str, strlen01 + 1, fmt, code);
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

    dbErrCode = sqlite3_prepare_v2(dbFile, sqldum, -1, &stmt, 0);
    dbErrCode = sqlite3_bind_text(stmt, 1, "meta", -1, 0);

    if((dbErrCode = sqlite3_step(stmt)) != SQLITE_ROW) {
        // TODO: Return here.
    }

    int val = sqlite3_column_int(stmt, 0);

    dbErrCode = sqlite3_finalize(stmt);

    return val;
}

/**
 * Create version 1 of database.
 */
static void createDbV1()
{
    char *sqldum;
    char *zErrMsg;

    // Create meta table.
    sqldum = "CREATE TABLE meta("
        " name TEXT NOT NULL,"
        " desc TEXT NOT NULL,"
        " value TEXT NOT NULL"
    ");";

    dbErrCode = sqlite3_exec(dbFile, sqldum, 0, 0, &zErrMsg);
    if (dbErrCode) {
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

    dbErrCode = sqlite3_exec(dbFile, sqldum, 0, 0, &zErrMsg);
    if (dbErrCode) {
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
    dbErrCode = sqlite3_exec(dbFile, sqldum, 0, 0, &zErrMsg);
    if (dbErrCode) {
        fprintf(stderr, "Error while creating \"items\" table: %s\n.", zErrMsg);
    }
    sqlite3_free(zErrMsg);
}

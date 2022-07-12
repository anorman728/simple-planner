#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "date-functions.h"
#include "planner-functions.h"

/** var dbFile Pointer to sqlite3 database object. */
static sqlite3 *dbFile;

static char saveNew(PlannerItem *item);
static char saveExisting(PlannerItem *item);
static int getFromWhere(PlannerItem **result, char *where, int *values, int count);
static int getFromWherePrepare(char *where, int *values, int count);


static char db_interface_build_err__db(char **str);
static char db_interface_build_err__ifce(char **str, int code);

static char updateDatabase();
static char doesDatabaseExist();
static char createDbV1();

/** This is the error code from SQLite. */
int dbRc = 0;

// Constants

const char DB_INTERFACE__OK = 0;

const char DB_INTERFACE__DB_ERROR = 1;

const char DB_INTERFACE__OUT_OF_MEMORY = 2;

const char DB_INTERFACE__CONT = 3;

const char DB_INTERFACE__PLANNER = 4;


/**
 * Open the database file.  Create one if it doesn't exist.  Returns RC from
 * constants.
 *
 * @param   filename    String representing filename path.
 */
char db_interface_initialize(char *filename)
{
    // https://sqlite.org/c3ref/open.html
    dbRc = sqlite3_open(filename, &dbFile);

    if (dbRc) {
        return DB_INTERFACE__DB_ERROR;
    }

    return updateDatabase();
}

/**
 * Close the database file.
 */
char db_interface_finalize()
{
    // https://sqlite.org/c3ref/close.html
    dbRc = sqlite3_close(dbFile);

    if (dbRc) {
        return DB_INTERFACE__DB_ERROR;
    }

    return DB_INTERFACE__OK;
}

/**
 * Save array of PlannerItem objects.
 *
 * @param   items   Array of items to save.
 */
char db_interface_save(PlannerItem *item)
{
    char rc;

    if (item->id == 0) {
        if ((rc = saveNew(item))) {
            return rc;
        }
    } else {
        if ((rc = saveExisting(item))) {
            return rc;
        }
    }

    return DB_INTERFACE__OK;
}

/**
 * Get the most recent error code from SQLite.
 *
 * NOTE: This doesn't seem to work for SQL misuse!  Making this function largely
 * untrustworthy.  Should refactor not to depend on the most recent error.
 */
int db_interface_get_db_err()
{
    return dbRc;
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
char db_interface_build_err(char **str, int code)
{
    // I decided to use heap memory for this because it makes the code
    // significantly easier and passing a function-scope pointer to this
    // function is not memory-safe and runs the risk of truncating the output
    // string.  There is also some precedent here, because this is also how
    // sqlite3_exec handles their errmsg string, if it's not null.  Having to
    // free the memory is worth it to have manageable code.  Can look at the
    // previous commit to see what it looked like before.

    if (code == DB_INTERFACE__DB_ERROR) {
        return db_interface_build_err__db(str);
    }

    return db_interface_build_err__ifce(str, code);

}

/**
 * Intentionally create a database error, for purposes of testing.
 */
void _db_interface_create_db_err()
{
    char *sqldum = "definitely not valid sql code";

    dbRc = sqlite3_exec(dbFile, sqldum, 0, 0, NULL);
}

/**
 * Retrieve single PlannerItem object.
 *
 * @param   result  Result passed back by argument.
 * @param   id      Id of row to retrieve.
 */
char db_interface_get(PlannerItem **result, int id)
{
    int vals[1];
    vals[0] = id;

    int rc = getFromWhere(result, "id = ?", vals, 1);

    if (rc != DB_INTERFACE__CONT && rc != DB_INTERFACE__OK) {
        return DB_INTERFACE__DB_ERROR;
    }

    rc = getFromWhere(result, "", vals, 0); // Just need to complete.

    if (rc) {
        // If it says "cont", that's still a problem in this case.
        // TODO: But I still need to handle that differently, because right now
        // it's pretty opaque and will be difficult to debug.
        return DB_INTERFACE__DB_ERROR;
    }

    return DB_INTERFACE__OK;
}

/**
 * Iterate through PlannerItem pointers from date range.  Returns
 * DB_INTERFACE__CONT when it successfully returns an item and DB_INTERFACE__OK
 * when the items to retrieve have been exhausted.
 *
 * @param   result  Result passed back by argument.
 * @param   lower   Lower bound (inclusive)
 * @param   upper   Upper bound (inclusive)
 */
char db_interface_range(PlannerItem **result, Date lower, Date upper)
{
    int vals[2];
    vals[0] = toInt(upper);
    vals[1] = toInt(lower);

    return getFromWhere(result, "date <= ? AND date >= ?", vals, 2);
}


// Static functions below this line.

/**
 * Save a new record to the database, and save the id.  Returns rc.
 *
 * @param   item    A PlannerItem pointer.
 */
static char saveNew(PlannerItem *item)
{
    char *insertRow = "INSERT INTO items(date, desc, rep, exp, done) \
        VALUES (?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt;

    if ((dbRc = sqlite3_prepare_v2(dbFile, insertRow, -1, &stmt, 0))) {
        return DB_INTERFACE__DB_ERROR;
    }

    int bindints[8];
    bindints[0] = 1;
    bindints[1] = toInt(item->date);

    bindints[2] = 3;
    bindints[3] = item->rep;

    bindints[4] = 4;
    bindints[5] = toInt(item->exp);

    bindints[6] = 5;
    bindints[7] = item->done;

    for (int i = 0; i < 8; i += 2) {
        if ((dbRc = sqlite3_bind_int(stmt, bindints[i], bindints[i+1]))) {
            return DB_INTERFACE__DB_ERROR;
        }
    }

    if ((dbRc = sqlite3_bind_text(stmt, 2, item->desc, -1, 0))) {
        return DB_INTERFACE__DB_ERROR;
    }

    if ((dbRc = sqlite3_step(stmt) != SQLITE_DONE)) {
        return DB_INTERFACE__DB_ERROR;
    }

    long iddum;
    iddum = sqlite3_last_insert_rowid(dbFile);

    item->id = iddum;

    if ((dbRc = sqlite3_finalize(stmt))) {
        return DB_INTERFACE__DB_ERROR;
    }

    return DB_INTERFACE__OK;
}

/**
 * Save an existing record to the database.
 *
 * @param   item    A PlannerItem pointer.
 */
static char saveExisting(PlannerItem *item)
{
    char *updateRow = "UPDATE items SET date = ?, desc = ?, rep = ?, exp = ?,"
     " done = ? WHERE id = ?;";

    sqlite3_stmt *stmt;

    if ((dbRc = sqlite3_prepare_v2(dbFile, updateRow, -1, &stmt, 0))) {
        return DB_INTERFACE__DB_ERROR;
    }

    int bindints[10];
    bindints[0] = 1;
    bindints[1] = toInt(item->date);

    bindints[2] = 3;
    bindints[3] = item->rep;

    bindints[4] = 4;
    bindints[5] = toInt(item->exp);

    bindints[6] = 5;
    bindints[7] = item->done;

    bindints[8] = 6;
    bindints[9] = item->id;

    for (int i = 0; i < 10; i += 2) {
        if ((dbRc = sqlite3_bind_int(stmt, bindints[i], bindints[i+1]))) {
            return DB_INTERFACE__DB_ERROR;
        }
    }

    if ((dbRc = sqlite3_bind_text(stmt, 2, item->desc, -1, 0))) {
        return DB_INTERFACE__DB_ERROR;
    }

    if ((dbRc = sqlite3_step(stmt) != SQLITE_DONE)) {
        return DB_INTERFACE__DB_ERROR;
    }

    if ((dbRc = sqlite3_finalize(stmt))) {
        return DB_INTERFACE__DB_ERROR;
    }

    return DB_INTERFACE__OK;
}

/** Module-scope object for use specifically with getFromWhere. */
sqlite3_stmt *stmtGfw = NULL;

/**
 * Iterate over PlannerItem pointers from where clause and the integers to be
 * bound to it.  (This assumes only integers will be bound.)
 *
 * Results dumped into `result` argument one at a time.  Returns
 * DB_INTERFACE__CONT if there are more results to be retrieved, or
 * DB_INTERFACE__OK when done.  Sets result to NULL if there's nothing to do.
 *
 * Helper method for db_interface_get and db_interface_range.
 *
 * @param   result  Result passed back by argument.
 * @param   where   WHERE clause, including ?s.
 * @param   values  Array of integers to bind.
 * @param   count   Number of integers to bind (i.e., count of "values").
 */
static int getFromWhere(PlannerItem **result, char *where, int *values, int count)
{
    int rc; // This is used in multiple contexts.

    if (stmtGfw == NULL) {
        if ((rc = getFromWherePrepare(where, values, count))) {
            return rc;
        }
    }

    dbRc = sqlite3_step(stmtGfw);

    if (dbRc == SQLITE_DONE) {
        // If SQLITE_DONE is returned, that means that it's *already* returned
        // the final row.
        result = NULL;
        dbRc = sqlite3_finalize(stmtGfw);
        stmtGfw = NULL;

        if (dbRc) {
            return DB_INTERFACE__DB_ERROR;
        }

        return DB_INTERFACE__OK;
    }

    if (dbRc != SQLITE_ROW) {
        return DB_INTERFACE__DB_ERROR;
    }

    rc = buildItem(
        result,
        sqlite3_column_int(stmtGfw, 0),
        toDate(sqlite3_column_int(stmtGfw, 1)),
        (char *) sqlite3_column_text(stmtGfw, 2),
        sqlite3_column_int(stmtGfw, 3),
        toDate(sqlite3_column_int(stmtGfw, 4)),
        sqlite3_column_int(stmtGfw, 5)
    );

    if (rc != PLANNER_STATUS__OK) {
        sqlite3_finalize(stmtGfw); // Don't bother with RC here.
        stmtGfw = NULL;
        freeItem(*result); // Result can't be trusted.
        result = NULL;
        return DB_INTERFACE__PLANNER;
    }

    return DB_INTERFACE__CONT;
}

/**
 * Prepare statement for getFromWhere.  Helper function for getFromWhere.
 *
 * @param   where   Directly passed by getFromWhere.
 * @param   values  Directly passed by getFromWhere.
 * @param   count   Directly passed by getFromWhere.
 */
static int getFromWherePrepare(char *where, int *values, int count)
{
    // Build SQL.

    char *sqldum = "SELECT id,date,desc,rep,exp,done FROM items WHERE ";
    // Not using "SELECT *" because the columns are only identified by
    // number, so explicitly naming the columns makes it more future-proof
    // and easier to update.

    char sql[strlen(sqldum) + strlen(where) + 2]; // Semicol and null term.
    strcpy(sql, sqldum);
    strcat(sql, where);
    strcat(sql, ";");

    // https://sqlite.org/c3ref/prepare.html
    if ((dbRc = sqlite3_prepare_v2(dbFile, sql, -1, &stmtGfw, 0))) {
        return DB_INTERFACE__DB_ERROR;
    }

    for (int i = 0; i < count; i++) {
        // https://sqlite.org/c3ref/bind_blob.html
        if ((dbRc = sqlite3_bind_int(stmtGfw, i + 1, values[i]))) {
            return DB_INTERFACE__DB_ERROR;
        }
    }

    return DB_INTERFACE__OK;
}

/**
 * Set error string from database error.
 *
 * @param   str     HEAP.  Pointer to string.
 * @param   code
 */
static char db_interface_build_err__db(char **str)
{
    int dbCode = sqlite3_errcode(dbFile);;
    char *fmt = "Database error: %d. ";

    int strlen01 = snprintf(NULL, 0, fmt, dbCode);

    char *dbMsg = (char *) sqlite3_errmsg(dbFile);

    int sizedum = strlen01 + strlen(dbMsg) + 1;
    *str = malloc(sizedum);

    if (str == NULL) {
        return DB_INTERFACE__OUT_OF_MEMORY;
    }

    snprintf(*str, sizedum, fmt, dbCode);
    strcat(*str, dbMsg);

    return DB_INTERFACE__OK;
}

/**
 * Set error string for interface error.
 *
 * @param   str     HEAP.  Pointer to string.
 * @param   code
 */
static char db_interface_build_err__ifce(char **str, int code)
{
    char *fmt = "Interface error: %d.";
    // TODO: May want to add strings to this, but not important enough at the
    // moment.  Right now, there are no interface errors anyway.
    int strlen01 = snprintf(NULL, 0, fmt, code);

    *str = malloc(strlen01 + 1);

    if (str == NULL) {
        return DB_INTERFACE__OUT_OF_MEMORY;
    }

    snprintf(*str, strlen01 + 1, fmt, code);

    return DB_INTERFACE__OK;
}


// Database update functions.

/**
 * Update database to latest version.
 */
static char updateDatabase()
{
    char rc; // Will be from this module's constants.  (Can return.)

    char tf;
    rc = doesDatabaseExist(&tf);

    if (rc) {
        return rc;
    }

    if (!tf) {
        rc = createDbV1();

        if (rc) {
            return rc;
        }
    }

    // When need newer database versions, will want some kind of "get database
    // version" function with cascading updates based on the version number.
    // The "value" in the database's meta table is text, though, so will need to
    // use one of those functions to convert c strings to int.  (Remember, can't
    // just do it through casting.)

    return DB_INTERFACE__OK;
}

/**
 * Check if there is a database to update.  Pointer is pointed to result.
 * Returns return code.
 *
 * @param   result
 */
static char doesDatabaseExist(char *result)
{
    sqlite3_stmt *stmt;

    char *sqldum = "SELECT count(*) as count FROM sqlite_master \
    WHERE type='table' AND name=?";

    dbRc = sqlite3_prepare_v2(dbFile, sqldum, -1, &stmt, 0);
    if (dbRc) {
        return DB_INTERFACE__DB_ERROR;
    }

    dbRc = sqlite3_bind_text(stmt, 1, "meta", -1, 0);

    if (dbRc) {
        return DB_INTERFACE__DB_ERROR;
    }

    dbRc = sqlite3_step(stmt);

    if(dbRc != SQLITE_ROW) {
        return DB_INTERFACE__DB_ERROR;
    }

    *result = sqlite3_column_int(stmt, 0);

    dbRc = sqlite3_finalize(stmt);

    if (dbRc) {
        return DB_INTERFACE__DB_ERROR;
    }

    return DB_INTERFACE__OK;
}

/**
 * Create version 1 of database.
 */
static char createDbV1()
{
    char *sqldum;

    // Create meta table.
    sqldum = "CREATE TABLE meta("
        " name TEXT NOT NULL,"
        " desc TEXT NOT NULL,"
        " value TEXT NOT NULL"
    ");";

    dbRc = sqlite3_exec(dbFile, sqldum, 0, 0, NULL);
    if (dbRc) {
        return DB_INTERFACE__DB_ERROR;
    }

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

    dbRc = sqlite3_exec(dbFile, sqldum, 0, 0, NULL);
    if (dbRc) {
        return DB_INTERFACE__DB_ERROR;
    }

    // Create planner_items table.
    sqldum = "CREATE TABLE items("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " date INTEGER NOT NULL,"
        " desc TEXT,"
        " rep INTEGER NOT NULL,"
        " exp INTEGER NOT NULL,"
        " done INTEGER NOT NULL"
    ");";
    dbRc = sqlite3_exec(dbFile, sqldum, 0, 0, NULL);
    if (dbRc) {
        return DB_INTERFACE__DB_ERROR;
    }

    return DB_INTERFACE__OK;
}

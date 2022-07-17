#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db-interface.h"
#include "date-functions.h"
#include "planner-functions.h"

/** var dbFile Pointer to sqlite3 database object. */
static sqlite3 *dbFile;

static char prepStat(char *str, sqlite3_stmt **stmtptr);
static char saveNew(PlannerItem *item);
static char saveExisting(PlannerItem *item);
static char getFromWhere(PlannerItem **result, char *where, int *values, int count);
static char getFromWherePrepare(char *where, int *values, int count);


static char db_interface_build_err__db(char **str);
static char db_interface_build_err__ifce(char **str, int code);


static char updateDatabase();
static char doesDatabaseExist();
static char createDbV1();

/** This is the error code from SQLite. */
int dbRc = 0;

/** This is the error code from Planner functions. */
char pfRc = 0;

// Internal-use-only macros.
#define RETURN_ERR_IF_APP(CODE, EXPR, RETVAL) \
    if ((CODE = (EXPR))) {\
        return (RETVAL); \
    }

/**
 * Open the database file.  Create one if it doesn't exist.  Returns RC from
 * constants.
 *
 * @param   filename    String representing filename path.
 */
char db_interface_initialize(char *filename)
{
    // https://sqlite.org/c3ref/open.html
    RETURN_ERR_IF_APP(
        dbRc,
        sqlite3_open(filename, &dbFile),
        DB_INTERFACE__DB_ERROR
    )

    return updateDatabase();
}

/**
 * Close the database file.
 */
char db_interface_finalize()
{
    // https://sqlite.org/c3ref/close.html
    RETURN_ERR_IF_APP(
        dbRc,
        sqlite3_close(dbFile),
        DB_INTERFACE__DB_ERROR
    );

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
        RETURN_ERR_IF_APP(rc, saveNew(item), rc)
    } else {
        RETURN_ERR_IF_APP(rc, saveExisting(item), rc)
    }

    return DB_INTERFACE__OK;
}

/**
 * Update description of a planner record by id.
 *
 * @param   id
 * @param   newdesc
 */
char db_interface_update_desc(long id, char *newdesc)
{
    char *updateRow = "UPDATE items SET desc = ? WHERE id = ?;";

    sqlite3_stmt *stmt;

    RETURN_ERR_IF_APP(
        dbRc,
        prepStat(updateRow, &stmt),
        DB_INTERFACE__DB_ERROR
    )

    RETURN_ERR_IF_APP(dbRc, sqlite3_bind_text(stmt, 1, newdesc, -1, 0),
        DB_INTERFACE__DB_ERROR)
    RETURN_ERR_IF_APP(dbRc, sqlite3_bind_int(stmt, 2, id),
        DB_INTERFACE__DB_ERROR)

    if ((dbRc = sqlite3_step(stmt)) != SQLITE_DONE) {
        return DB_INTERFACE__DB_ERROR;
    }

    RETURN_ERR_IF_APP(dbRc, sqlite3_finalize(stmt), DB_INTERFACE__DB_ERROR)

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
 * @param   str     SETS HEAP.  Pointer to string.
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

    if (code == DB_INTERFACE__PLANNER) {
        return planner_functions_build_err(str, pfRc);
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

    if (*result != NULL) {
        // If it's null, then nothing was found, so don't need to complete the
        // statement.
        RETURN_ERR_IF_APP(
            rc,
            getFromWhere(NULL, "", vals, 0),
            DB_INTERFACE__DB_ERROR
        )
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
 * Helper function for dealing with preparing statements.
 *
 * @param   string
 * @param   stmtptr
 */
static char prepStat(char *str, sqlite3_stmt **stmtptr)
{
    return sqlite3_prepare_v2(dbFile, str, -1, stmtptr, 0);
}

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

    RETURN_ERR_IF_APP(dbRc, prepStat(insertRow, &stmt), DB_INTERFACE__DB_ERROR)

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
        RETURN_ERR_IF_APP(dbRc, sqlite3_bind_int(stmt, bindints[i], bindints[i+1]),
            DB_INTERFACE__DB_ERROR)
    }

    RETURN_ERR_IF_APP(dbRc, sqlite3_bind_text(stmt, 2, item->desc, -1, 0),
        DB_INTERFACE__DB_ERROR);

    if ((dbRc = sqlite3_step(stmt) != SQLITE_DONE)) {
        return DB_INTERFACE__DB_ERROR;
    }

    long iddum;
    iddum = sqlite3_last_insert_rowid(dbFile);

    item->id = iddum;

    RETURN_ERR_IF_APP(dbRc, sqlite3_finalize(stmt), DB_INTERFACE__DB_ERROR)

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

    RETURN_ERR_IF_APP(dbRc, prepStat(updateRow, &stmt), DB_INTERFACE__DB_ERROR)

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
        RETURN_ERR_IF_APP(dbRc, sqlite3_bind_int(stmt, bindints[i], bindints[i+1]),
            DB_INTERFACE__DB_ERROR)
    }

    RETURN_ERR_IF_APP(dbRc, sqlite3_bind_text(stmt, 2, item->desc, -1, 0),
        DB_INTERFACE__DB_ERROR)

    if ((dbRc = sqlite3_step(stmt) != SQLITE_DONE)) {
        return DB_INTERFACE__DB_ERROR;
    }

    RETURN_ERR_IF_APP(dbRc, sqlite3_finalize(stmt), DB_INTERFACE__DB_ERROR);

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
 * @param   result
 *  Result passed back by argument.  If this is null (not what it points to),
 *  then this was intentionally passed just to close the argument.
 * @param   where
 *  WHERE clause, including ?s.
 * @param   values
 *  Array of integers to bind.
 * @param   count
 *  Number of integers to bind (i.e., count of "values").
 */
static char getFromWhere(PlannerItem **result, char *where, int *values, int count)
{
    char rc;

    if (stmtGfw == NULL) {
        RETURN_ERR_IF_APP(rc, getFromWherePrepare(where, values, count), rc)
    }

    dbRc = sqlite3_step(stmtGfw);

    if (dbRc == SQLITE_DONE) {
        // If SQLITE_DONE is returned, that means that it's *already* returned
        // the final row.
        // Note that if SQLITE_DONE is returned on first row, that means that
        // there were no records found, so the *first* result is null.
        if (result != NULL) {
            // If result *itself* is null, then that was intentionally passed
            // just to close the statement.
            *result = NULL;
        }
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

    if (result == NULL) {
        // This means a null pointer was passed when it wasn't ready to close.
        return DB_INTERFACE__INTERNAL;
    }

    pfRc = buildItem(
        result,
        sqlite3_column_int(stmtGfw, 0),
        toDate(sqlite3_column_int(stmtGfw, 1)),
        (char *) sqlite3_column_text(stmtGfw, 2),
        sqlite3_column_int(stmtGfw, 3),
        toDate(sqlite3_column_int(stmtGfw, 4)),
        sqlite3_column_int(stmtGfw, 5)
    );

    if (pfRc != PLANNER_STATUS__OK) {
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
static char getFromWherePrepare(char *where, int *values, int count)
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

    RETURN_ERR_IF_APP(dbRc, prepStat(sql, &stmtGfw), DB_INTERFACE__DB_ERROR)

    for (int i = 0; i < count; i++) {
        // https://sqlite.org/c3ref/bind_blob.html
        RETURN_ERR_IF_APP(dbRc, sqlite3_bind_int(stmtGfw, i + 1, values[i]),
            DB_INTERFACE__DB_ERROR);
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
    char *fmt = "Database error: %d. ";

    int strlen01 = snprintf(NULL, 0, fmt, dbRc);

    int dbCode = sqlite3_errcode(dbFile);
    // This is not reliable-- In some cases, it will return zero, so...

    char *dbMsg;
    if (dbCode) {
        // ...if it's nonzero, we go ahead and use it, because the message is
        // more detailed this way, but...
        dbMsg = (char *) sqlite3_errmsg(dbFile);
    } else {
        // ...if it's zero, we use the description according to the code.
        dbMsg = (char *) sqlite3_errstr(dbRc);
    }

    int sizedum = strlen01 + strlen(dbMsg) + 1;
    *str = malloc(sizedum);

    if (str == NULL) {
        return DB_INTERFACE__OUT_OF_MEMORY;
    }

    snprintf(*str, sizedum, fmt, dbRc);
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
    char *strdum;

    switch (code) {
        case DB_INTERFACE__OK:
            strdum = "No error for db interface.";
            break;
        case DB_INTERFACE__OUT_OF_MEMORY:
            strdum = "Out of memory or db interface.";
            break;
        case DB_INTERFACE__CONT:
            strdum = "No error: Continue.";
            break;
        case DB_INTERFACE__INTERNAL:
            strdum = "Coding problem inside this class.";
            break;
        default:
            strdum = "Unknown error for db interface.";
    }

    *str = malloc(strlen(strdum) + 1);

    if (str == NULL) {
        return DB_INTERFACE__OUT_OF_MEMORY;
    }

    strcpy(*str, strdum);

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

    RETURN_ERR_IF_APP(rc, doesDatabaseExist(&tf), rc);

    if (!tf) {
        RETURN_ERR_IF_APP(rc, createDbV1(), rc);
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

    RETURN_ERR_IF_APP(dbRc, prepStat(sqldum, &stmt), DB_INTERFACE__DB_ERROR)

    RETURN_ERR_IF_APP(dbRc, sqlite3_bind_text(stmt, 1, "meta", -1, 0),
        DB_INTERFACE__DB_ERROR)

    dbRc = sqlite3_step(stmt);

    if(dbRc != SQLITE_ROW) {
        return DB_INTERFACE__DB_ERROR;
    }

    *result = sqlite3_column_int(stmt, 0);

    RETURN_ERR_IF_APP(dbRc, sqlite3_finalize(stmt), DB_INTERFACE__DB_ERROR)

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

    RETURN_ERR_IF_APP(dbRc, sqlite3_exec(dbFile, sqldum, 0, 0, NULL),
        DB_INTERFACE__DB_ERROR)

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

    RETURN_ERR_IF_APP(dbRc, sqlite3_exec(dbFile, sqldum, 0, 0, NULL),
        DB_INTERFACE__DB_ERROR)

    // Create planner_items table.
    sqldum = "CREATE TABLE items("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " date INTEGER NOT NULL,"
        " desc TEXT,"
        " rep INTEGER NOT NULL,"
        " exp INTEGER NOT NULL,"
        " done INTEGER NOT NULL"
    ");";
    RETURN_ERR_IF_APP(dbRc, sqlite3_exec(dbFile, sqldum, 0, 0, NULL),
        DB_INTERFACE__DB_ERROR)

    return DB_INTERFACE__OK;
}

#include <sqlite3.h>
#include <stdio.h>

/** var dbFile Pointer to sqlite3 database object. */
static sqlite3 *dbFile;

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
    int rc = sqlite3_open(filename, &dbFile);

    if (rc) {
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

// Static functions below this line.


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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "date-functions.h"

void testYearsToDays();
void testMonthsToDays();

// Forward declarations for helper functions.
void testToInt();
void testToString();
void testToDate();
void testToIntErrorHandling();
void testToIntErrorHandlingHelper();
void testGetWeekday();
void testGetWeek();
void testDateMatch();
void testDatepp();
void testDatemm();

int main()
{
    testToInt();
    testToString();
    testToIntErrorHandling();
    testGetWeekday();
    testGetWeek();
    testDateMatch();
    testDatepp();
    testDatemm();
}

/**
 * Test both toInt and toDate.
 */
void testToInt()
{
    printf("...Starting testToInt.\n");

    struct tm baseDate = {};
    baseDate.tm_year = 101;
    baseDate.tm_mon = 0;
    //baseDate.tm_mday = 0;
    baseDate.tm_hour = 6; // To avoid DST nonsense.

    struct tm dumDate;

    int date;
    Date testObj;

    for (int i = 0; i < 37200; i++) { // 37200 = 100 years (technically well over).
        dumDate = baseDate;
        dumDate.tm_mday = i;
        mktime(&dumDate);

        testObj.year = dumDate.tm_year - 99;
        testObj.month = dumDate.tm_mon;
        testObj.day = dumDate.tm_mday - 1;

        date = toInt(testObj);
        if (date == -1) {
            printf("FAILURE Date result is invalid.\n");
            return;
        }

        Date resultObj;
        resultObj = toDate(date);

        // TODO: Better messages here.  (Can use asctime with dumDate.)
        if (resultObj.year != testObj.year) {
            printf("FAILURE Year should be %d, but found %d.\n", testObj.year, resultObj.year);
        }

        if (resultObj.month != testObj.month) {
            printf("FAILURE Month should be %d, but found %d.\n", testObj.month, resultObj.month);
        }

        if (resultObj.day != testObj.day) {
            printf("FAILURE Day should be %d, but found %d.\n", testObj.day, resultObj.day);
        }
    }

    printf("...Finished testToInt.\n");
}

/**
 * Test toString.
 */
void testToString()
{
    printf("...Starting testToString.\n");

    Date testObj = buildDate(3, 5, 21); // June 22, 2004.

    char *result;

    int rc = toString(&result, testObj);

    if (rc) {
        printf("ERROR: Received error: %d\n", rc);
        return;
    }

    if (strcmp(result, "2004-06-22") != 0) {
        printf("FAILURE: Found %s\n.", result);
    }

    free(result);

    printf("...Finished testToString.\n");
}

/**
 * Test error handling in toInt.
 */
void testToIntErrorHandling()
{
    // I'd like to redirect stderr so it doesn't show anything at all while the
    // test is running, but at the moment I don't want to figure out how to do
    // it.  When I learn the more "proper" way to do things, I might end up
    // getting rid of this altogether.

    printf("...Starting testToIntErrorHandling.\n");

    // Test that negative day, month, or year returns zero.
    Date dateObj = {};

    dateObj.day = -3; // Tests with -1 could possibly get through errors in code.
    testToIntErrorHandlingHelper("neg day", dateObj);

    dateObj.day = 0;
    dateObj.month = -1;
    testToIntErrorHandlingHelper("neg month", dateObj);

    dateObj.month = 0;
    dateObj.year = -1;
    testToIntErrorHandlingHelper("neg year", dateObj);

    dateObj.year = 0;

    // Test ranges.
    dateObj.day = 32;
    testToIntErrorHandlingHelper("day range", dateObj);

    dateObj.day = 0;
    dateObj.month = 12;
    testToIntErrorHandlingHelper("month range", dateObj);

    dateObj.month = 1;
    dateObj.year = 5772805;
    testToIntErrorHandlingHelper("year range", dateObj);

    printf("...Finished testToIntErrorHandling.\n");
}

/**
 * Test getWeekday function.
 */
void testGetWeekday()
{
    printf("...Starting testGetWeekday.\n");

    Date dateObj = {};
    dateObj.day = 17;
    dateObj.month = 2;
    dateObj.year = 21;

    int res;

    if ((res = getWeekday(dateObj)) != 5) {
        printf("FAILURE expected 5 (Friday), but found %d.\n", res);
    }

    printf("...Finished testGetWeekday.\n");
}

/**
 * Test dateMatch function.
 */
void testDateMatch()
{
    printf("...Starting testDateMatch.\n");

    Date dateObj1 = buildDate(19, 11, 24); // Christmas 2020.
    Date dateObj2 = buildDate(19, 11, 24); // Same day.

    if (!dateMatch(&dateObj1, &dateObj2)) {
        printf("FAILURE: Dates that were the same returned false.\n");
    }

    dateObj2.day = 22; // Changed to Festivus.

    if (dateMatch(&dateObj1, &dateObj2)) {
        printf("FAILURE: Dates that were different returned true.\n");
    }

    printf("...Finished testDateMatch.\n");
}

/**
 * Test getWeek function.
 */
void testGetWeek()
{
    printf("...Starting testGetWeek.\n");

    Date dateObj = buildDate(23,10, 28);
    // 2024/11/29, which is Friday, so should get back the 24th (23).

    Date res = getWeek(dateObj);

    if (res.day != 23) {
        printf("FAILURE: expected 23, but found %d\n", res.day);
    }

    // Just for good measure-- Check the string.
    char *resdesc;
    toString(&resdesc, res);
    //printf("results: %s\n", resdesc);
    if (strcmp(resdesc, "2024-11-24") != 0) {
        printf("FAILURE: Date does not match expected 2024-11-24.  Found %s.\n", resdesc);
    }
    free(resdesc);

    // Now test on week with a month break.

    dateObj = buildDate(23,9,3);
    // 2024/10/4, which is a Friday, should go back to 2024/9/29.

    res = getWeek(dateObj);

    if (res.day != 28) {
        printf("FAILURE: Expected 28, but found %d\n.", res.day);
    }

    printf("...Finished testGetWeek.\n");
}

/**
 * Test datepp function.
 */
void testDatepp()
{
    printf("...Starting testDatepp.\n");

    Date dateObj;
    char *strDum;
    char *expDum;
    char *failString = "FAILURE: %s should result in %s but found %s.\n";

    // Generic.

    dateObj = buildDate(23, 7, 15); // Aug 16, 2024

    datepp(&dateObj);
    toString(&strDum, dateObj);
    expDum = "2024-08-17";

    if (strcmp(expDum, strDum) != 0) {
        printf(failString, "Generic", expDum, strDum);
    }
    free(strDum);

    // Month that only has 30 days.
    dateObj = buildDate(23, 5, 29); // Jun 30, 2024

    datepp(&dateObj);
    toString(&strDum, dateObj);
    expDum = "2024-07-01";

    if (strcmp(expDum, strDum) != 0) {
        printf(failString, expDum, strDum);
    }
    free(strDum);

    // Month that has 31 days
    dateObj = buildDate(23, 6, 30); // Jul 31, 2024.

    datepp(&dateObj);
    toString(&strDum, dateObj);
    expDum = "2024-08-01";

    if (strcmp(expDum, strDum) != 0) {
        printf(failString, "31-days", expDum, strDum);
    }
    free(strDum);

    // Feb non-leap-year.
    dateObj = buildDate(22, 1, 27); // Feb 28, 2023.

    datepp(&dateObj);
    toString(&strDum, dateObj);
    expDum = "2023-03-01";

    if (strcmp(expDum, strDum) != 0) {
        printf(failString, "Non-leap year", expDum, strDum);
    }
    free(strDum);


    // Feb leap year
    // 28 to 29.
    dateObj = buildDate(23, 1, 27); // Feb 28, 2024

    datepp(&dateObj);
    toString(&strDum, dateObj);
    expDum = "2024-02-29";

    if (strcmp(expDum, strDum) != 0) {
        printf(failString, "First leap year", expDum, strDum);
    }
    free(strDum);

    // Now 29 to March.
    datepp(&dateObj);
    toString(&strDum, dateObj);
    expDum = "2024-03-01";

    if (strcmp(expDum, strDum) != 0) {
        printf(failString, "Second leap year", expDum, strDum);
    }
    free(strDum);

    // End of year.
    dateObj = buildDate(23,11,30); // Dec 31, 2024

    datepp(&dateObj);
    toString(&strDum, dateObj);
    expDum = "2025-01-01";

    if (strcmp(expDum, strDum) != 0) {
        printf(failString, "End of year", expDum, strDum);
    }
    free(strDum);

    printf("...Finished testDatepp.\n");
}

/**
 * Test datemm function.
 */
void testDatemm()
{
    printf("...Starting testDatemm.\n");

    Date dateObj;
    char *strDum;
    char *expDum;
    char *failString = "FAILURE: %s should result in %s but found %s.\n";

    // Generic

    dateObj = buildDate(23, 7, 15); // Aug 16, 2024.

    datemm(&dateObj);
    toString(&strDum, dateObj);
    expDum = "2024-08-15";

    if(strcmp(expDum, strDum) != 0) {
        printf(failString, "Generic", expDum, strDum);
    }
    free(strDum);

    // Previous year.

    dateObj = buildDate(23, 0, 0); // Jan 1, 2024.

    datemm(&dateObj);
    toString(&strDum, dateObj);
    expDum = "2023-12-31";

    if (strcmp(expDum, strDum) != 0) {
        printf(failString, "Previous year", expDum, strDum);
    }
    free(strDum);

    // March on leap year.

    dateObj = buildDate(23, 2, 0); // Mar 1, 2024

    datemm(&dateObj);
    toString(&strDum, dateObj);
    expDum = "2024-02-29";

    if (strcmp(expDum, strDum) != 0) {
        printf(failString, "March on leap year", expDum, strDum);
    }
    free(strDum);

    // March on non-leap year.

    dateObj = buildDate(22, 2, 0); // Mar 1, 2023

    datemm(&dateObj);
    toString(&strDum, dateObj);
    expDum = "2023-02-28";

    if (strcmp(expDum, strDum) != 0) {
        printf(failString, "March on non-leap year", expDum, strDum);
    }
    free(strDum);

    // 31-day months.
    dateObj = buildDate(22, 3, 0); // April 1, 2023

    datemm(&dateObj);
    toString(&strDum, dateObj);
    expDum = "2023-03-31";

    if (strcmp(expDum, strDum) != 0) {
        printf(failString, "31-day months", expDum, strDum);
    }
    free(strDum);

    // 30-day months.
    dateObj = buildDate(22, 6, 0); // July 1, 2023

    datemm(&dateObj);
    toString(&strDum, dateObj);
    expDum = "2023-06-30";

    if (strcmp(expDum, strDum) != 0) {
        printf(failString, "30-day months", expDum, strDum);
    }
    free(strDum);

    printf("...Finished testDatemm.\n");
}


// Helper functions below this line.

/**
   Helper function for testToIntErrorHandling.
 */
void testToIntErrorHandlingHelper(char * errPrint, Date dateObj)
{
    int res;

    fprintf(stderr, "...This should be error for");
    fprintf(stderr, " %s: ", errPrint);
    if ((res = toInt(dateObj)) != -1) {
        printf("FAILURE Result is not -1 for %s.  Found %d.\n", errPrint, res);
    }
}

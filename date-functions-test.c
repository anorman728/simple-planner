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

int main()
{
    testToInt();
    testToString();
    testToIntErrorHandling();
    testGetWeekday();
    testGetWeek();
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

    printf("...Finished testGetWeek.\n");
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

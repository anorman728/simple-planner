#include <stdio.h>
#include <time.h>

#include "date-functions.h"

void testYearsToDays();
void testMonthsToDays();

// Forward declarations for helper functions.
void testToInt();
void testToIntErrorHandling();
void testToIntErrorHandlingHelper();

int main()
{
    testToInt();
    testToIntErrorHandling();
}

/**
 * Test both toInt and toDate.
 */
void testToInt()
{
    printf("Starting testToInt.\n");

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

    printf("Finished testToInt.\n");
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

    printf("Starting testToIntErrorHandling.\n");

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

    printf("Finished testToIntErrorHandling.\n");
}


// Helper functions below this line.

/**
   Helper function for testToIntErrorHandling.
 */
void testToIntErrorHandlingHelper(char * errPrint, Date dateObj)
{
    int res;

    fprintf(stderr, "This should be error for");
    fprintf(stderr, " %s: ", errPrint);
    if ((res = toInt(dateObj)) != -1) {
        printf("FAILURE Result is not -1 for %s.  Found %d.\n", errPrint, res);
    }
}

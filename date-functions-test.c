#include <stdio.h>
#include <time.h>

#include "date-functions.h"

void testYearsToDays();
void testMonthsToDays();

void testIntConvert();

int main()
{
    testIntConvert();
}

void testIntConvert()
{
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

        Date resultObj;
        resultObj = toDate(date);

        // TODO: Better messages here.  (Can use asctime with dumDate.)
        if (resultObj.year != testObj.year) {
            printf("Year should be %d, but found %d.\n", testObj.year, resultObj.year);
        }

        if (resultObj.month != testObj.month) {
            printf("Month should be %d, but found %d.\n", testObj.month, resultObj.month);
        }

        if (resultObj.day != testObj.day) {
            printf("Day should be %d, but found %d.\n", testObj.day, resultObj.day);
        }
    }
}

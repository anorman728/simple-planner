#include <stdio.h>

#include "date-functions.h"

void testYearsToDays();
void testMonthsToDays();

int main()
{
    testYearsToDays();
    testMonthsToDays();
}

void testYearsToDays()
{
    Date dateObj = {};
    dateObj.month = 0;
    dateObj.day = 0;

    int result;

    int yearexp[24] = { // year / exp pairs.
        2000, 0,
        2001, 366,
        2002, 731,
        2003, 1096,
        2004, 1461,
        2005, 1827,
        2006, 2192,
        2007, 2557,
        2008, 2922,
        2009, 3288,
        2010, 3653,
        2011, 4018
    };
    // This is a poor man's associative array...  Or rather a destitute man's
    // associative array.

    for (int i = 0; i < 24; i += 2) {
        dateObj.year = yearexp[i] - 2000;
        result = YearsToDays(dateObj);

        if (result != yearexp[i+1]) {
            printf("Wrong result for %d: %d\n", yearexp[i], result);
        }
    }
}

void testMonthsToDays()
{
    // TODO
}

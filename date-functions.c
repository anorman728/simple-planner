#include <stdio.h>

#include "date-functions.h"

// I needed to make this because the `struct tm` object was causing more
// problems than it solves.  I do not want to have to work around DST or time
// zones and I don't need anything more precise than days of the year.

static int monthdays(int mon, int isleap);

/**
 * Convert a Date object to an integer, being the number of days since Jan 1,
 * 2000.
 *
 * @param   dateObj
 */
int toInt(Date dateObj)
{
    int total = 0;

    total+= YearsToDays(dateObj);
    total+= MonthsToDays(dateObj);
    total+= dateObj.day;

    return total;
}

/**
 * Count up the number of days that have passed when reach a given year.
 *
 * @param   dateObj
 */
int YearsToDays(Date dateObj)
{
    if (dateObj.year == 0) {
        return 0;
    }

    int total = 365 * dateObj.year;

    // Add in leap years.
    int leapYears = (dateObj.year - 1) / 4 + 1;
    // Number of *passed* leap years, so need to subtract one.

    total+= leapYears;

    return total;
}

/**
 * Count up the number of days that have passed in a year when reached a given
 * month.
 *
 * @param   dateObj
 */
int MonthsToDays(Date dateObj)
{
    int total = 0;

    int mon = dateObj.month;
    while (mon != 0) {
        total+= monthdays(--mon, (dateObj.year % 4) == 0);
    }

    return total;
}


// Helper functions below this line.

/**
 * Return the number of days for a given month.
 *
 * @param   mon     The month, 0-11.
 * @param   isleap  1 if leapyear.
 */
static int monthdays(int mon, int isleap)
{
    switch (mon) {
        case 1:
            return (isleap ? 29 : 28);
        case 3:
        case 5:
        case 8:
        case 10:
            return 30;
        default:
            return 31;
    }
}


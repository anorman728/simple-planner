#include <stdio.h>

#include "date-functions.h"

// I needed to make this because the `struct tm` object was causing more
// problems than it solves.  I do not want to have to work around DST or time
// zones and I don't need anything more precise than days of the year.

/**
 * The maximum number of days in a month, and a modulo base we use for
 * conversions to integers.
 */
const int DAYMOD = 31;

/**
 * The maximum number of months in a year, and a factor in the modulo base we
 * use for conversion to integers.
 */
const int MONTHMOD = 12;

/**
 * Convert a Date object to an integer.
 *
 * Integer is not human-readable as a date, but it orders the same way.
 *
 * @param   dateObj
 */
int toInt(Date dateObj)
{
    // These integers work by a kind of dual-modular reduction.
    // There are, at most, 31 days in a month.  That means it's mod 31, so 0 -
    // 30.  If the number is 31, that means we're on the first day of the
    // second month.  Similarly, there are 12 months in a year, but our base is
    // days, so there are 12 * 31 = 372 "days" in a year (counting invalid days
    // like Feb 30th).  So that means that if we're on day 373, we know we're on
    // the first day of the second year.

    //return dateObj.day + DAYMOD * dateObj.month + (MONTHMOD * DAYMOD) * dateObj.year;
    // Keeping commented version because it's clearer than below.
    return dateObj.day + DAYMOD * (dateObj.month + MONTHMOD * dateObj.year);
}

/**
 * Convert an integer back to a Date object.
 *
 * @param   dateInt
 */
Date toDate(int dateInt)
{
    // See comment in toInt to see how this works.  Deconstructing is basic
    // number theory.

    Date dateObj = {};

    dateObj.day = dateInt % DAYMOD;
    dateObj.month = (dateInt - dateObj.day) / DAYMOD % MONTHMOD;
    dateObj.year = (dateInt - dateObj.day - DAYMOD * dateObj.month) / (MONTHMOD * DAYMOD);

    return dateObj;
}

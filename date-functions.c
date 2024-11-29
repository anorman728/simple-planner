#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#include "date-functions.h"
#include "planner-functions.h"

// I needed to make this because the `struct tm` object was causing more
// problems than it solves.  I do not want to have to work around DST or time
// zones and I don't need anything more precise than days of the year.
// `struct tm` is actually used, though, within the static functions, to be
// able to find things like the the weekday, the date range of a week, etc.

/**
 * The maximum number of days in a month, and a modulo base we use for
 * conversions to and from integers.
 */
#define DAYMOD 31

/**
 * The maximum number of months in a year, and a factor in a modulo base we
 * use for conversion to and from integers.
 */
#define MONTHMOD 12

/**
 * The number of days in a year under this system.  Not 365 or 366, but the
 * multibase system (described in toInt) which results in 31 * 12 = 372 days.
 */
#define DAYSINYEAR 372

/** The minimum size that this module needs an `int` type to be (in bytes). */
#define MININTSIZE 4


// Forward declarations for helper functions.
static char toIntErrorHandling(Date dateObj);
static struct tm dateToTm(Date dateObj);
//static Date tmToDate(struct tm tmObj);

/**
 * Build a date from the year, month, and day.
 *
 * @param   yr  Year, count since 2001 (i.e., 2013 is 12).
 * @param   mn  Month, 0-11
 * @param   dy  Day of month, 0 - 30.
 */
Date buildDate(int yr, int mn, int dy)
{
    Date returnVal = {};
    returnVal.year = yr;
    returnVal.month = mn;
    returnVal.day = dy;

    return returnVal;
}

/**
 * Convert a Date object to an integer.  Returns negative one on error.
 *
 * Note that due to -1 being error, it is not compatible with dates before 2001.
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

    if (toIntErrorHandling(dateObj)) {
        return -1;
    }

    //return dateObj.day + DAYMOD * dateObj.month + (MONTHMOD * DAYMOD) * dateObj.year;
    // Keeping commented version because it's clearer than below.
    return dateObj.day + DAYMOD * (dateObj.month + MONTHMOD * dateObj.year);
}

/**
 * Convert Date object to a string in form Y-m-d.  Passed back by argument.
 * RETURNS OBJECT ON HEAP!
 *
 * @param   ret
 * @param   dateObj
 */
int toString(char **ret, Date dateObj)
{
    // Don't forget the null terminator!
    *ret = malloc(11 * (sizeof ret));

    if (*ret == NULL) {
        return DATE_FUNCTIONS__OUT_OF_MEMORY;
    }

    sprintf(*ret, "%04d-%02d-%02d", (dateObj.year + 2001), dateObj.month + 1, dateObj.day + 1);

    return DATE_FUNCTIONS__OK;
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
    dateObj.year = (dateInt - dateObj.day - DAYMOD * dateObj.month) / (DAYSINYEAR);

    return dateObj;
}

/**
 * Find the weekday of a date object.
 *
 * @param   dateObj
 */
int getWeekday(Date dateObj)
{
    struct tm dumval = dateToTm(dateObj);

    return dumval.tm_wday;
}

// TODO: Get weekday desc. (Mon, Tue, etc.)
// TODO: Get single-char weekday. (M, T, etc.)
// Note: Above two might be outside the scope of this module.  Maybe
// something closer to the user should handle it.
// Update: ATM, I'm thinking this is going to be handled by
// planner-interface.c, and probably not at all the way this describes.

/**
 * Reduce the date (in integer form) based on its repetition type.
 *
 * @param   dateInt
 * @param   rep
 */
int reduceIntDate(int dateInt, char rep)
{
    switch (rep) {
        case REP_YEARLY:
            return dateInt % DAYSINYEAR;
        default:
            return dateInt;
    }
}


// Static functions below this line.

/**
 * Check the dateObj in toInt for errors, to make sure to crash early.
 * Returns true if there are errors.
 *
 * @param   dateObj
 */
static char toIntErrorHandling(Date dateObj)
{
    if (sizeof(int) < MININTSIZE) {
        fprintf(stderr, "date-functions.toInt: This platform is not" \
        " compatible with this module.  Requires int size to be at least" \
        " %d\n", MININTSIZE);
        return 1;
    }

    if (dateObj.day < 0 || dateObj.month < 0 || dateObj.year < 0) {
        fprintf(stderr, "Cannot convert negative values into integer for" \
        " d/m/y.  Found %d/%d/%d.\n", dateObj.day, dateObj.month, dateObj.year);
        return 1;
    }

    if (dateObj.day > 30) {
        fprintf(stderr, "Day out of range.  Found %d.\n", dateObj.day);
        return 1;
    }
    if (dateObj.month > 11) {
        fprintf(stderr, "Month out of range.  Found %d.\n", dateObj.month);
        return 1;
    }
    if (dateObj.year > 5772804) {
        fprintf(stderr, "Year is out of range.  Found %d.  And there's no" \
        " way this program is still going to be used in this year anyway.\n",
        dateObj.year);
        return 1;
    }

    return 0;
}

/**
 * Convert a Date object to a struct tm object.
 *
 * @param   dateObj
 */
static struct tm dateToTm(Date dateObj)
{
    struct tm returnVal = {};

    returnVal.tm_mday      = dateObj.day + 1;
    returnVal.tm_mon       = dateObj.month;
    returnVal.tm_year      = dateObj.year + 101;
    returnVal.tm_isdst     = -1;

    mktime(&returnVal);

    return returnVal;
}

/**
 * Convert a struct tm object to a Date object.
 *
 * @param   tmObj
 */
// Currently commented out because it's not being used and it's static.
//static Date tmToDate(struct tm tmObj)
//{
//    Date returnVal = {};
//
//    returnVal.day   = tmObj.tm_mday - 1;
//    returnVal.month = tmObj.tm_mon;
//    returnVal.year  = tmObj.tm_year - 101;
//
//    return returnVal;
//}

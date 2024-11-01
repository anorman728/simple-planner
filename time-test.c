#include <stdio.h>
#include <time.h>

// Stuff for testing time functionality.
// https://en.cppreference.com/w/c/chrono/tm

// Regarding DST:  The tm struct correctly handles DST, but it does not
// automatically enable or disable it for the time!  Or rather, it does, but in
// a way that I don't want it to.  It's very strange (to me, at least).  If you
// create a tm in July at midnight, but don't set tm_isdst = 1, then it will say
// it's 1 am.  This can be "disabled" for lack of a better word by setting
// tm_isdst to a negative value.  In my case, where I only care about the day
// and not the hour, I'm going to keep it that way.  There is debate on whether
// or not that's a good idea.
// Honestly, if I were to deal with calculations as precise as an hour and I
// needed to deal with DST, I don't think I would use this, because it doesn't
// take time zones into account.  Some time zones don't do DST, and I think some
// international ones have different start and end dates.

int main(int argc, char *argv[])
{
    struct tm dateObj = {};
    // If the brackets aren't here, will initialize to garbage.

    dateObj.tm_year = 103; // Number of years since 1900.
    dateObj.tm_mon = 6; // This is JULY, because months start at zero.
    dateObj.tm_mday = 4; // But days don't.

    mktime(&dateObj); // Recalculates everything in the struct.

    printf("%s\n", asctime(&dateObj)); // "asctime" makes it kinda/sorta readable.

    // I can add days, etc.  It's actually easier here than in php.
    dateObj.tm_mday += 365;
    mktime(&dateObj);
    printf("%s\n", asctime(&dateObj)); // Jul 3, because of leap year.

    // Also note that tm_wday is days since Sunday.
}

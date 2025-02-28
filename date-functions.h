#ifndef date_functions_h
#define date_functions_h

#include <time.h>

// Constants

#define DATE_FUNCTIONS__OK              0
#define DATE_FUNCTIONS__OUT_OF_MEMORY   1

typedef struct date_obj {
    /** @var Number of complete years since the year 2001. (Like 2003 would be 2.)*/
    int year;

    /** @var Month number, 0 - 11. */
    int month;

    /** @var Day of month, 0 - 30. */
    int day;
} Date;

Date buildDate(int yr, int mn, int dy);

int toInt(Date dateObj);

int toString(char **ret, Date dateObj);

Date toDate(int dateInt);

int getWeekday(Date dateObj);

Date getWeek(Date dateObj);

int reduceIntDate(int dateInt, char rep);

Date tmToDate(struct tm tmObj);

Date todayDate();

char dateMatch(Date *dayOne, Date *dayTwo);

void datepp(Date *dateObj);

void datemm(Date *dateObj);

#endif

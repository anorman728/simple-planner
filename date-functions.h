#ifndef date_functions_h
#define date_functions_h

typedef struct date_obj {
    /** @var Number of complete years since the year 2001. */
    int year;

    /** @var Month number, 0 - 11. */
    int month;

    /** @var Day of month, 0 - 30. */
    int day;
} Date;

int toInt(Date dateObj);

Date toDate(int dateInt);

int getWeekday(Date dateObj);

#endif

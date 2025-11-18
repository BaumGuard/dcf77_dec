#ifndef UTILS_H
#define UTILS_H

/*
Turn a number (weekday) into the corresponding weekday
abbreviation (str)
1 -> "Mon"
2 -> "Tue"
3 -> "Wed"
4 -> "Thu"
5 -> "Fri"
6 -> "Sat"
7 -> "Sun"
*/
void Weekday_Abbr ( int weekday, char* str );

/*
Check if a a given value (value) is equal to (compare) +- threshold
*/
bool In_Range ( int value, int compare, int threshold );

#endif

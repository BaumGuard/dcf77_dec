#include "utils.h"

#include <stdlib.h>
#include <stdio.h>

/*---------------------------------------------------------------*/

bool In_Range ( int value, int compare, int threshold ) {
    return abs(value-compare) <= threshold;
} /* In_Range () */

/*---------------------------------------------------------------*/

void Weekday_Abbr ( int weekday, char* str ) {
    switch ( weekday ) {
        case 1:
            sprintf( str, "Mon" );
            break;
        case 2:
            sprintf( str, "Tue" );
            break;
        case 3:
            sprintf( str, "Wed" );
            break;
        case 4:
            sprintf( str, "Thu" );
            break;
        case 5:
            sprintf( str, "Fri" );
            break;
        case 6:
            sprintf( str, "Sat" );
            break;
        case 7:
            sprintf( str, "Sun" );
            break;
    }
} /* Weekday_Abbr () */

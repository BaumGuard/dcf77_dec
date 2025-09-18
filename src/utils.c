#include "utils.h"

/*-----------------------------------------------------------*/

void Progress_Bar ( int bits_received, int bits_total ) {
    int
        full =  ((double) bits_received /
            (double) bits_total) * PROGRESS_BAR_LENGTH,
        empty = ((double)(bits_total-bits_received) /
            (double)bits_total) * PROGRESS_BAR_LENGTH;

    fprintf( stderr, "\r[" );
    for ( int i=0; i<full; i++ ) {
        fprintf( stderr, "#" );
    }

    for ( int i=0; i<empty; i++ ) {
        fprintf( stderr, " " );
    }

    fprintf( stderr, "] Bit %d/%d", bits_received, bits_total );
    fflush( stderr );
} /* Progress_Bar() */

/*-----------------------------------------------------------*/

bool Is_In_Range ( uint32_t value, uint32_t compare, uint32_t tolerance ) {
    if ( value >= compare-tolerance && value <= compare+tolerance ) {
        return true;
    }
    return false;
} /* Is_In_Range() */


/*-----------------------------------------------------------*/


void Print_DCF77_Data ( DCF77_Data* data ) {

    char yes_no [2][4] = {
        "No\0",
        "Yes\0"
    };

    char weekdays [8][10] = {
        "\0",
        "Monday\0",
        "Tuesday\0",
        "Wednesday\0",
        "Thursday\0",
        "Friday\0",
        "Saturday\0",
        "Sunday\0"
    };

    char dst_string [5];
    if ( data->dst ) {
        strcpy( dst_string, "MEST\0" );
    }
    else {
        strcpy( dst_string, "MET\0" );
    }

    printf( "\n\n============================================\n" );
    printf( "Date and time:\n" );
    printf(
        "\t%02d.%02d.%02d %02d:%02d (%s) - %s\n\n",
        data->day,
        data->month,
        data->year,
        data->hour,
        data->minute,
        dst_string,
        weekdays[data->day_of_week]
    );
    printf( "DST starts after this hour: %s\n", yes_no[data->dst_starts] );
    printf( "Leap second after this hour: %s\n", yes_no[data->leap_second_next_hour] );
    printf( "============================================\n\n" );
} /* Print_DCF77_Data() */

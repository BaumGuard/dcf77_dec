#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define PROGRESS_BAR_LENGTH 30

/*-----------------------------------------------------------*/

typedef struct {
    uint8_t
        minute,
        hour,
        day,
        month,
        year,
        day_of_week;

    bool
        dst_starts,
        dst,
        leap_second_next_hour;
} DCF77_Data;


/*-----------------------------------------------------------*/

/*
Display a progress bar to show the number of received bits
per cycle

Args:
 - bits_received : Number of bits received so far
 - bits_total    : Total number of bits
*/
void Progress_Bar ( int bits_received, int bits_total );

/*-----------------------------------------------------------*/

/*
Check if a value is within the tolerance range

Args:
 - value     : Value to check whether it is within the
               tolerance range
 - compare   : Value to compore 'value' to
 - tolerance : Tolerance radius around 'value'
*/
bool Is_In_Range ( uint32_t value, uint32_t compare, uint32_t tolerance );


/*-----------------------------------------------------------*/


/*
Print the decoded DCF77 data to the console

Args:
 - data : DCF77 data as a DCF77_Data object
*/
void Print_DCF77_Data ( DCF77_Data* data );

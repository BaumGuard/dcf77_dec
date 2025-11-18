#include "decoder.h"

#include "common.h"
#include "buffer.h"
#include "input.h"
#include "demod.h"
#include "filter.h"
#include "event.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

/*---------------------------------------------------------------*/

int
    sync_n_samples,
    bit0_n_samples,
    bit1_n_samples;

/*---------------------------------------------------------------*/

/*
Get the level threshold in the processed signal at which a sample
should be classified as LOW or HIGH.

This is achieved by reading 2 seconds of audio from the buffer
and extracting the lowest and highest value.
After that the average of the lowest and highest value is calculated.
From the average we subtract 20 % of the average because then the
bit recognition works more reliable.

The global threshold variable is then set to the this average.
*/
void Get_Level_Threshold () {
    size_t len = 2 * SAMPLE_RATE;

    int16_t min_value = Get_Next_Value();
    int16_t max_value = Get_Next_Value();

    int16_t value;

    for ( size_t i = 2; i < len; i++ ) {
        value = Get_Next_Value();

        if ( value < min_value ) {
            min_value = value;
        }
        if ( value > max_value ) {
            max_value = value;
        }
    }

    data.bit_threshold = (int16_t) ( ((int)min_value + (int)max_value) / 2 );
    data.bit_threshold -= (int16_t) ( data.bit_threshold * 0.2 );
} /* Get_Level_Threshold () */

/*---------------------------------------------------------------*/

/*
Wait for the sync signal (1.8 s HIGH)
*/
void Wait_For_Sync () {
    int16_t value;
    int sample_count = 0;

    while ( lpf_running ) {
        while (
            (value = Get_Next_Value()) > data.bit_threshold &&
            lpf_running
        ) {
            sample_count++;
        }

        if ( sample_count >= sync_n_samples ) return;
        sample_count = 0;
    }
} /* Wait_For_Sync () */

/*---------------------------------------------------------------*/

/*
Get the next bit from the stream and return it

0.1 s LOW -> Bit 0
0.2 s LOW -> Bit 1
*/
int Get_Next_Bit () {
    int sample_count = 0;
    int16_t value;

    while ( lpf_running ) {
        while (
            (value = Get_Next_Value()) < data.bit_threshold &&
            lpf_running
        ) {
            sample_count++;
        }

        if ( In_Range(sample_count, bit0_n_samples, TOLERANCE) ) {
            data.bit_count++;
            Post_Event( UPDATE_PROGRESS );
            if ( data.bit_count == 58 ) data.bit_count = 0;
            return 0;
        }
        if ( In_Range(sample_count, bit1_n_samples, TOLERANCE) ) {
            data.bit_count++;
            Post_Event( UPDATE_PROGRESS );
            if ( data.bit_count == 58 ) data.bit_count = 0;
            return 1;
        }
    }

    return -1;
} /* Get_Next_Bit () */

/*---------------------------------------------------------------*/

void* Decode_DCF77 ( void* arg ) {

    sync_n_samples = (int) ( SYNC_DURATION * SAMPLE_RATE );
    bit0_n_samples = (int) ( BIT0_DURATION * SAMPLE_RATE );
    bit1_n_samples = (int) ( BIT1_DURATION * SAMPLE_RATE );

    Get_Level_Threshold();
    Post_Event( THRESHOLD_AVAILABLE );

    uint8_t bits [59];

    data.data_correct = true;

    while ( lpf_running ) {
        data.bit_count = 0;

        Wait_For_Sync();
        Post_Event( SYNC_AVAILABLE );

        // Idle bits (Bit 0-15)
        for ( size_t i = 0; i <= 15; i++ ) {
            bits[i] = Get_Next_Bit();
        }

        // DST starts at the end of next hour (Bit 16)
        bits[16] = Get_Next_Bit();
        data.dst_at_the_end_of_hour = bits[16];
        Post_Event( DST_STARTS_AVAILABLE );

        // DST (Bit 17-18)
        // 10 -> DST
        // 01 -> No DST
        bits[17] = Get_Next_Bit();
        bits[18] = Get_Next_Bit();
        if ( bits[17] && !bits[18] ) {
            data.dst = true;
        }
        else if ( !bits[17] && bits[18] ) {
            data.dst = false;
        }
        else {
            data.data_correct = false;
        }
        Post_Event( DST_AVAILABLE );

        // Leap second at the end of this hour (Bit 19)
        bits[19] = Get_Next_Bit();
        data.leap_second = bits[19];
        Post_Event( LEAP_SECOND_AVAILABLE );

        // Start of time information (Bit 20)
        // Should be 1
        bits[20] = Get_Next_Bit();
        if ( !bits[20] ) {
            data.data_correct = false;
        }

        // Minute (Bit 21-27)
        for ( size_t i = 21; i <= 27; i++ ) {
            bits[i] = Get_Next_Bit();
        }

        data. minute =
             1 * bits[21] +  2 * bits[22] +  4 * bits[23] + 8 * bits[24] +
            10 * bits[25] + 20 * bits[26] + 40 * bits[27];

        Post_Event( MINUTE_AVAILABLE );

        // Minute parity
        // Sum of all minute bits mod 2
        uint8_t minute_parity =
            bits[21] + bits[22] + bits[23] + bits[24] +
            bits[25] + bits[26] + bits[27];

        bits[28] = Get_Next_Bit();
        data.minute_correct = minute_parity % 2 == bits[28];

        Post_Event( MINUTE_PARITY_AVAILABLE );

        if ( !data.minute_correct )
            data.data_correct = false;

        // Hour (Bit 29-34)
        for ( size_t i = 29; i <= 34; i++ ) {
            bits[i] = Get_Next_Bit();
        }

        data.hour =
             1 * bits[29] +  2 * bits[30] +  4 * bits[31] + 8 * bits[32] +
            10 * bits[33] + 20 * bits[34];
        Post_Event( HOUR_AVAILABLE );

        // Hour parity
        // Sum of all hour bits mod 2
        uint8_t hour_parity =
            bits[29] + bits[30] + bits[31] + bits[32] +
            bits[33] + bits[34];

        bits[35] = Get_Next_Bit();
        data.hour_correct = hour_parity % 2 == bits[35];
        Post_Event( HOUR_PARITY_AVAILABLE );

        if ( !data.hour_correct )
            data.data_correct = false;

        uint8_t date_parity = 0;

        // Day (Bit 36-41)
        for ( size_t i = 36; i <= 41; i++ ) {
            bits[i] = Get_Next_Bit();
            date_parity += bits[i];
        }

        data.day =
             1 * bits[36] +  2 * bits[37] +  4 * bits[38] + 8 * bits[39] +
            10 * bits[40] + 20 * bits[41];
        Post_Event( DAY_AVAILABLE );

        // Day of the week (Bit 42-44)
        for ( size_t i = 42; i <= 44; i++ ) {
            bits[i] = Get_Next_Bit();
            date_parity += bits[i];
        }

        data.week_day =
             1 * bits[42] +  2 * bits[43] +  4 * bits[44];

        Post_Event( WEEKDAY_AVAILABLE );


        // Month (Bit 45-49)
        for ( size_t i = 45; i <= 49; i++ ) {
            bits[i] = Get_Next_Bit();
            date_parity += bits[i];
        }

        data.month =
             1 * bits[45] +  2 * bits[46] +  4 * bits[47] + 8 * bits[48] +
            10 * bits[49];

        Post_Event( MONTH_AVAILABLE );


        // Year (Bit 50-57)
        for ( size_t i = 50; i <= 57; i++ ) {
            bits[i] = Get_Next_Bit();
            date_parity += bits[i];
        }

        data.year =
             1 * bits[50] +  2 * bits[51] +  4 * bits[52] +  8 * bits[53] +
            10 * bits[54] + 20 * bits[55] + 40 * bits[56] + 80 * bits[57];

        Post_Event( YEAR_AVAILABLE );


        bits[58] = Get_Next_Bit();

        // Date parity
        // Sum of the bits for the day, day of week, month and year mod 2
        data.date_correct = date_parity % 2 == bits[58];
        Post_Event( DATE_PARITY_AVAILABLE );


        if ( !data.date_correct )
            data.data_correct = false;

        Post_Event( RECEPTION_CYCLE_END );
    }

    return NULL;
} /* Decode_DCF77 () */

/*---------------------------------------------------------------*/

void* Start_Processing ( void* arg ) {
    input_index = 0;
    envelope_index = 0;
    proc_index = 0;
    lpf_index = 0;
    buf_index = 0;

    pthread_t
        input_thread,
        envelope_thread,
        lpf_thread,
        proc_thread;

    pthread_mutex_init( &buf_mutex, NULL );
    pthread_cond_init( &input_data_available, NULL );
    pthread_cond_init( &envelope_data_available, NULL );
    pthread_cond_init( &lpf_data_available, NULL );

    pthread_create( &input_thread, NULL, Alsa_Capture, NULL );
    pthread_create( &envelope_thread, NULL, Envelope_Detection, NULL );
    pthread_create( &lpf_thread, NULL, LPF, NULL );
    pthread_create( &proc_thread, NULL, Decode_DCF77, NULL );

    pthread_join( input_thread, NULL );
    pthread_join( envelope_thread, NULL );
    pthread_join( lpf_thread, NULL );
    pthread_join( proc_thread, NULL );

    pthread_mutex_destroy( &buf_mutex );
    pthread_cond_destroy( &input_data_available );
    pthread_cond_destroy( &envelope_data_available );
    pthread_cond_destroy( &lpf_data_available );

    return NULL;
} /* Start_Processing () */

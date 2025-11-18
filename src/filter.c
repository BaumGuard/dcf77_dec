#include "common.h"
#include "filter.h"

#include "buffer.h"

#include <pthread.h>
#include <math.h>
#include <stdio.h>

/*---------------------------------------------------------------*/

float
    b0, b1, b2,
    a1, a2;

/*---------------------------------------------------------------*/

void Calculate_Butterworth_LPF_IIF_Coeffs (
    float cutoff_freq, float sample_freq
) {
    float ff = cutoff_freq / sample_freq;

    const float ita = 1.0/ tan( M_PI * ff );
    const float q = sqrt( 2.0 );
    b0 = 1.0 / (1.0 + q*ita + ita*ita);
    b1 = 2 * b0;
    b2 = b0;
    a1 = 2.0 * (ita*ita - 1.0) * b0;
    a2 = -(1.0 - q*ita + ita*ita) * b0;
} /* Calculate_Butterworth_LPF_IIF_Coeffs () */

/*---------------------------------------------------------------*/

void* LPF ( void* arg ) {
    Calculate_Butterworth_LPF_IIF_Coeffs( 100.0, 48000.0 );
    float
        x0 = 0.0, x1 = 0.0, x2 = 0.0,
        y0 = 0.0, y1 = 0.0, y2 = 0.0;

    while ( !envelope_running );
    lpf_running = true;

    while ( envelope_running ) {
        pthread_mutex_lock( &buf_mutex );
        while ( envelope_index <= lpf_index && envelope_running ) {
            pthread_cond_wait( &envelope_data_available, &buf_mutex );
        }

        for ( size_t i = 0; i < BUFFER_SIZE; i++ ) {
            x2 = x1;
            x1 = x0;
            x0 = (float) buf[lpf_index % NUM_BUFFERS][i];

            y2 = y1;
            y1 = y0;
            y0 = b0 * x0 + b1 * x1 + b2 * x2 + a1 * y1 + a2 * y2;

            buf[lpf_index % NUM_BUFFERS][i] = (int16_t) y0;
        }
        lpf_index++;

        pthread_cond_signal( &lpf_data_available );
        pthread_mutex_unlock( &buf_mutex );
    }
    lpf_running = false;
    pthread_cond_broadcast( &lpf_data_available );

    return NULL;
} /* LPF () */

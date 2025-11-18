#include "common.h"
#include "demod.h"

#include "buffer.h"

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

/*---------------------------------------------------------------*/

void* Envelope_Detection ( void* arg ) {
    while ( !input_running );
    envelope_running = true;

    while ( input_running ) {
        pthread_mutex_lock( &buf_mutex );
        while ( input_index <= envelope_index && input_running ) {
            pthread_cond_wait( &input_data_available, &buf_mutex );
        }

        for ( size_t i = 0; i < BUFFER_SIZE; i++ ) {
            buf[envelope_index % NUM_BUFFERS][i] =
                abs( buf[envelope_index % NUM_BUFFERS][i] );
        }
        envelope_index++;

        pthread_cond_signal( &envelope_data_available );
        pthread_mutex_unlock( &buf_mutex );
    }
    envelope_running = false;
    pthread_cond_broadcast( &envelope_data_available );

    return NULL;
} /* Envelope_Detection () */

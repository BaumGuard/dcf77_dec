#include "buffer.h"

#include "common.h"

#include <pthread.h>
#include <stdio.h>

/*---------------------------------------------------------------*/

int16_t buf [NUM_BUFFERS][BUFFER_SIZE];

/*---------------------------------------------------------------*/

int16_t Get_Next_Value () {
    pthread_mutex_lock( &buf_mutex );
    while (
        buf_index == 0 && lpf_index <= proc_index &&
        decoder_running
    ) {
        pthread_cond_wait( &lpf_data_available, &buf_mutex );
    }

    int16_t value = buf[proc_index % NUM_BUFFERS][buf_index];

    pthread_mutex_unlock( &buf_mutex );

    buf_index++;

    if ( buf_index == BUFFER_SIZE ) {
        proc_index++;
        buf_index = 0;
    }

    return value;
} /* Get_Next_Value () */

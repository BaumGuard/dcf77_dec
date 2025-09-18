#include <pthread.h>

#include "common.h"
#include "sample.h"
#include "ring_buffer.h"
#include "input.h"
#include "decoder.h"

/*-----------------------------------------------------------*/

int main () {
    sample_data_type = S16_LE;
    sample_rate = 48000;

    Ring_Buffer
        raw_samples,
        goertzel_values;

    Init_Ring_Buffer( &raw_samples, 1024 );
    Init_Ring_Buffer( &goertzel_values, 1024 );

    pthread_t
        input_thread,
        read_samples_thread,
        goertzel_thread,
        decoder_thread;


    pthread_cond_init( &goertzel_wait, NULL );
    pthread_cond_init( &capture_wait, NULL );
    pthread_mutex_init( &capture_mutex, NULL );
    pthread_mutex_init( &sample_reader_mutex, NULL );

    Thread_Data
        sample_reader_data = { .in_buffer = NULL, .out_buffer = &raw_samples },
        goertzel_data = { .in_buffer = &raw_samples, .out_buffer = &goertzel_values },
        decoder_data = { .in_buffer = &goertzel_values, .out_buffer = NULL, .opt_buffer = &raw_samples };

    pthread_create( &input_thread, NULL, Alsa_Capture, NULL );
    pthread_create( &read_samples_thread, NULL, Read_Samples, (void*)&sample_reader_data );
    pthread_create( &goertzel_thread, NULL, Goertzel, (void*)&goertzel_data );
    pthread_create( &decoder_thread, NULL, Decode_DCF77, (void*)&decoder_data );

    pthread_join( input_thread, NULL );
    pthread_join( read_samples_thread, NULL );
    pthread_join( goertzel_thread, NULL );
    pthread_join( decoder_thread, NULL );


    pthread_mutex_destroy( &capture_mutex );
    pthread_mutex_destroy( &sample_reader_mutex );
    pthread_cond_destroy( &capture_wait );
    pthread_cond_destroy( &goertzel_wait );
    Destroy_Buffer( &raw_samples );
    Destroy_Buffer( &goertzel_values );

    return 0;
}

#include "input.h"

#include "common.h"
#include "buffer.h"

#include <alsa/asoundlib.h>
#include <stdio.h>

/*---------------------------------------------------------------*/

void* Alsa_Capture ( void* arg ) {
    int err;
    snd_pcm_t* capture_handle;
    snd_pcm_hw_params_t* hw_params;

    err = snd_pcm_open( &capture_handle, "pulse", SND_PCM_STREAM_CAPTURE, 0 );
    if ( err < 0 ) {
        fprintf( stderr, "ALSA: Cannot openn audio device\n" );
        return NULL;
    }

    err = snd_pcm_hw_params_malloc( &hw_params );
    if ( err < 0 ) {
        fprintf( stderr, "ALSA: Cannot allocate hardware parameter structure\n" );
        return NULL;
    }

    err = snd_pcm_hw_params_any( capture_handle, hw_params );
    if ( err < 0 ) {
        fprintf( stderr, "ALSA: Cannot initialize hardware parameter structure\n" );
        return NULL;
    }

    err = snd_pcm_hw_params_set_access( capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED );
    if ( err < 0 ) {
        fprintf( stderr, "ALSA: Cannot set access type\n" );
        return NULL;
    }

    err = snd_pcm_hw_params_set_format( capture_handle, hw_params, SND_PCM_FORMAT_S16_LE );
    if ( err < 0 ) {
        fprintf( stderr, "ALSA: Cannot set sample format\n" );
        return NULL;
    }

    unsigned int rate = 48000;
    err = snd_pcm_hw_params_set_rate_near( capture_handle, hw_params, &rate, 0 );
    if ( err < 0 ) {
        fprintf( stderr, "ALSA: Cannot set sample rate\n" );
        return NULL;
    }

    err = snd_pcm_hw_params_set_channels( capture_handle, hw_params, 1 );
    if ( err < 0 ) {
        fprintf( stderr, "ALSA: Cannot set channel count\n" );
        return NULL;
    }

    err = snd_pcm_hw_params( capture_handle, hw_params );
    if ( err < 0 ) {
        fprintf( stderr, "ALSA: Cannot set parameters\n" );
        return NULL;
    }

    snd_pcm_hw_params_free( hw_params );


    err = snd_pcm_prepare( capture_handle );
    if ( err < 0 ) {
        fprintf( stderr, "ALSA: Cannot prepare audio interface for use\n" );
        return NULL;
    }

    input_running = true;
    while ( err != 128 && decoder_running ) {
        err = snd_pcm_readi( capture_handle, (void*)buf[input_index % NUM_BUFFERS], BUFFER_SIZE );
        input_index++;

        pthread_cond_signal( &input_data_available );
    }
    input_running = false;
    pthread_cond_broadcast( &input_data_available );

    snd_pcm_close( capture_handle );

    return NULL;
} /* Alsa_Capture() */

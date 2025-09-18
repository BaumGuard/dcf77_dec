#pragma once

#include <alsa/asoundlib.h>
#include <pthread.h>

#include "sample.h"
#include "common.h"

/*-----------------------------------------------------------*/

/*
Capture PCM data from an ALSA audio device
Default device: pulse
*/
void* Alsa_Capture ( void* arg );

/*-----------------------------------------------------------*/

/*
Capture PCM data from stdin
*/
void* Stdin_Capture( void* arg );

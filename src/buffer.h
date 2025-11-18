#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>

#include "buffer.h"

#define NUM_BUFFERS 8
#define BUFFER_SIZE 16384

// Sample ring buffer
extern int16_t buf [NUM_BUFFERS][BUFFER_SIZE];

/*
Get the next sample value from the ring buffer
*/
int16_t Get_Next_Value ();

#endif

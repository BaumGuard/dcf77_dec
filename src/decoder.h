#pragma once

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>

#include "common.h"
#include "ring_buffer.h"
#include "sample.h"
#include "utils.h"

#define PAUSE_BIT_0 0.1
#define PAUSE_BIT_1 0.2
#define SYNC_DURATION 1.8

#define GOERTZEL_BLOCK_SIZE 1000
#define BLOCK_TOLERANCE 4

#define TWO_PI 6.283185307
#define INT24_MAX 8388607


/*-----------------------------------------------------------*/

typedef struct {
    Ring_Buffer* in_buffer;
    Ring_Buffer* out_buffer;
    Ring_Buffer* opt_buffer;
} Thread_Data;

enum sample_types {
    S8,
    S16_LE,
    S24_LE,
    S32_LE,
    U8,
    U16_LE,
    U24_LE,
    U32_LE,

    S16_BE,
    S24_BE,
    S32_BE,
    U16_BE,
    U24_BE,
    U32_BE,

    F32,
    F64
};


/*-----------------------------------------------------------*/









/*-----------------------------------------------------------*/

/*
Find the frequency of the DCF77 signal using DFT

Args:
 - buf : Ring buffer with the PCM values
*/
void Find_Signal_Frequency ( Ring_Buffer* buf );

/*-----------------------------------------------------------*/

/*
Goerzel's algorithm for a block of PCM values to find
the magnitude of a given frequency in the block

 - in_buffer  : PCM values
 - out_buffer : Frequency magnitudes for each block
*/
void* Goertzel ( void* arg );


/*-----------------------------------------------------------*/

/*
Wait for the sync signal (1.8 s long carrier)

 - buf : Ring buffer with the Goertzel values
*/
void Find_Sync ( Ring_Buffer* buf );

/*-----------------------------------------------------------*/

/*
Get the next bit from the signal based on the pause length

Pauses:
 - 100 ms -> Bit 0
 - 200 ms -> Bit 1

Args:
 - buf : Ring buffer with the Goertzel values
*/
uint8_t Get_Next_Bit ( Ring_Buffer* buf );


/*-----------------------------------------------------------*/

/*
Decode the DCF77 signal

Args:
 - in_buffer : Ring buffer with the Goertzel values
*/
void* Decode_DCF77 ( void* ptr );


/*-----------------------------------------------------------*/

/*
Read the PCM values from stdin, convert them to double and
write them into a ring buffer

 - out_buffer : PCM values converted to double
*/
void* Read_Samples ( void* ptr );

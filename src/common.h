
#include <pthread.h>

#include "sample.h"

#ifndef COMMON_H
#define COMMON_H

extern pthread_cond_t goertzel_wait;
extern pthread_mutex_t sample_reader_mutex;
extern pthread_mutex_t capture_mutex;
extern pthread_cond_t capture_wait;

extern int data_available;

extern Sample sample;

extern int sample_size;
extern bool is_little_endian;

extern uint32_t sample_rate;
extern int signal_freq;
extern double threshold_bit_1;
extern int sample_data_type;

extern bool signal_freq_found;

#endif

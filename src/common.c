#include "common.h"

pthread_cond_t goertzel_wait;
pthread_mutex_t sample_reader_mutex;
pthread_mutex_t capture_mutex;
pthread_cond_t capture_wait;

int data_available = 0;

Sample sample;

int sample_size;
bool is_little_endian;

uint32_t sample_rate;
int signal_freq;
double threshold_bit_1;
int sample_data_type;

bool signal_freq_found = false;

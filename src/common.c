#include "common.h"

struct DCF77_Data data;

bool start_button_pressed = false;

bool decoder_running = false;

pthread_t
    processing_thread,
    event_thread;

pthread_cond_t
    input_data_available,
    envelope_data_available,
    lpf_data_available;

pthread_mutex_t
    buf_mutex;

uint32_t
    input_index     = 0,
    envelope_index  = 0,
    lpf_index       = 0,
    proc_index      = 0,
    buf_index       = 0;

bool
    input_running    = false,
    envelope_running = false,
    lpf_running      = false;

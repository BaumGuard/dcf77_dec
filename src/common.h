#ifndef COMMON_H
#define COMMON_H

#include <pthread.h>
#include <stdint.h>

// Structure for storing decoded DCF77 data
struct DCF77_Data {
    bool dst_at_the_end_of_hour;
    bool dst;
    bool leap_second;

    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t week_day;
    uint8_t month;
    uint8_t year;

    bool minute_correct;
    bool hour_correct;
    bool date_correct;

    bool data_correct;

    int bit_threshold;
    int bit_count;
};

// Global instance of DCF77_Data used by the decoder
// and the event handler
extern struct DCF77_Data data;

extern bool decoder_running;

extern bool start_button_pressed;

extern pthread_t
    processing_thread,
    event_thread;

extern pthread_cond_t
    input_data_available,
    envelope_data_available,
    lpf_data_available;

extern pthread_mutex_t
    buf_mutex;

extern uint32_t
    input_index,
    envelope_index,
    lpf_index,
    proc_index,
    buf_index;

extern bool
    input_running,
    envelope_running,
    lpf_running;

#endif

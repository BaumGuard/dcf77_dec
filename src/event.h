#ifndef EVENT_H
#define EVENT_H

#include <pthread.h>

// Event types
enum EventTypes {
    MINUTE_AVAILABLE,
    HOUR_AVAILABLE,
    DAY_AVAILABLE,
    MONTH_AVAILABLE,
    YEAR_AVAILABLE,
    WEEKDAY_AVAILABLE,

    DST_AVAILABLE,
    DST_STARTS_AVAILABLE,
    LEAP_SECOND_AVAILABLE,

    MINUTE_PARITY_AVAILABLE,
    HOUR_PARITY_AVAILABLE,
    DATE_PARITY_AVAILABLE,

    SYNC_AVAILABLE,
    THRESHOLD_AVAILABLE,

    UPDATE_PROGRESS,

    RECEPTION_CYCLE_END
};

extern pthread_cond_t
    event_available,
    event_processed;

/*
Report a new event to the event handler
*/
void Post_Event ( int _event_type );

/*
Handle incoming events by manipulating the GUI
*/
void* Gui_Handle_Event ( void* arg );

#endif

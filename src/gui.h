#ifndef GUI_H
#define GUI_H

#include "decoder.h"

#include <gtk/gtk.h>

// C objects for the GUI Objects in the Glade file
extern GtkWidget
    *day_field,
    *month_field,
    *year_field,
    *hour_field,
    *minute_field,
    *dst_field,
    *weekday_field,
    *dst_start_field,
    *leap_second_field,
    *start_button,
    *progress,
    *progress_field,
    *sample_rate_field,
    *channels_field,
    *sample_type_field,
    *threshold_field,
    *sync_field,
    *day_status_field,
    *month_status_field,
    *year_status_field,
    *hour_status_field,
    *minute_status_field,
    *dayofweek_status_field;

/*
Construct the GUI from the Glade file and launch it
*/
void* Launch_Gui ( void* ptr );

#endif

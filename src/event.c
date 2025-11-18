#include "event.h"

#include "gui.h"
#include "utils.h"
#include "common.h"

//#include <pthread.h>

/*---------------------------------------------------------------*/

bool event_received = false;
int event_type;

pthread_mutex_t event_mutex;

pthread_cond_t
    event_available,
    event_processed;

/*---------------------------------------------------------------*/

void Post_Event ( int _event_type ) {
    pthread_mutex_lock( &event_mutex );
    while ( event_received && decoder_running ) {
        pthread_cond_wait( &event_processed, &event_mutex );
    }

    event_received = true;
    event_type = _event_type;

    pthread_cond_signal( &event_available );
    pthread_mutex_unlock( &event_mutex );
} /* Post_Event () */

/*---------------------------------------------------------------*/

void* Gui_Handle_Event ( void* arg ) {

    pthread_mutex_init( &event_mutex, NULL );
    pthread_cond_init( &event_available, NULL );
    pthread_cond_init( &event_processed, NULL );

    char label_str [100];

    while ( decoder_running ) {
        pthread_mutex_lock( &event_mutex );
        while ( !event_received && decoder_running ) {
            pthread_cond_wait( &event_available, &event_mutex );
        }

        switch ( event_type ) {
            case MINUTE_AVAILABLE:
                sprintf( label_str, "%02d", data.minute );
                gtk_label_set_text( (GtkLabel*)minute_field, label_str );
                break;

            case HOUR_AVAILABLE:
                sprintf( label_str, "%02d", data.hour );
                gtk_label_set_text( (GtkLabel*)hour_field, label_str );
                break;

            case DAY_AVAILABLE:
                sprintf( label_str, "%02d", data.day );
                gtk_label_set_text( (GtkLabel*)day_field, label_str );
                break;

            case MONTH_AVAILABLE:
                sprintf( label_str, "%02d", data.month );
                gtk_label_set_text( (GtkLabel*)month_field, label_str );
                break;

            case YEAR_AVAILABLE:
                sprintf( label_str, "%02d", data.year );
                gtk_label_set_text( (GtkLabel*)year_field, label_str );
                break;

            case WEEKDAY_AVAILABLE:
                Weekday_Abbr( data.week_day, label_str );
                gtk_label_set_text( (GtkLabel*)weekday_field, label_str );
                break;

            case DST_AVAILABLE:
                sprintf( label_str, "%s", data.dst ? "✓" : "⛌" );
                gtk_label_set_text( (GtkLabel*)dst_field, label_str );
                break;

            case DST_STARTS_AVAILABLE:
                sprintf( label_str, "%s", data.dst_at_the_end_of_hour ? "✓" : "⛌" );
                gtk_label_set_text( (GtkLabel*)dst_start_field, label_str );
                break;

            case LEAP_SECOND_AVAILABLE:
                sprintf( label_str, "%s", data.leap_second ? "✓" : "⛌" );
                gtk_label_set_text( (GtkLabel*)leap_second_field, label_str );
                break;

            case MINUTE_PARITY_AVAILABLE:
                if ( data.minute_correct ) {
                    sprintf( label_str, "<span foreground=\"green\">%02d</span>", data.minute );
                }
                else {
                    sprintf( label_str, "<span foreground=\"red\">%02d</span>", data.minute );
                }
                gtk_label_set_markup( (GtkLabel*)minute_field, label_str );
                break;

            case HOUR_PARITY_AVAILABLE:
                if ( data.hour_correct ) {
                    sprintf( label_str, "<span foreground=\"green\">%02d</span>", data.hour );
                }
                else {
                    sprintf( label_str, "<span foreground=\"red\">%02d</span>", data.hour );
                }
                gtk_label_set_markup( (GtkLabel*)hour_field, label_str );
                break;

            case DATE_PARITY_AVAILABLE:
                if ( data.date_correct ) {
                    sprintf( label_str, "<span foreground=\"green\">%02d</span>", data.day );
                    gtk_label_set_markup( (GtkLabel*)day_field, label_str );
                    Weekday_Abbr( data.week_day, label_str );
                    sprintf( label_str, "<span foreground=\"green\">%s</span>", label_str );
                    gtk_label_set_markup( (GtkLabel*)weekday_field, label_str );
                    sprintf( label_str, "<span foreground=\"green\">%02d</span>", data.month );
                    gtk_label_set_markup( (GtkLabel*)month_field, label_str );
                    sprintf( label_str, "<span foreground=\"green\">%02d</span>", data.year );
                    gtk_label_set_markup( (GtkLabel*)year_field, label_str );
                }
                else {
                    sprintf( label_str, "<span foreground=\"red\">%02d</span>", data.day );
                    gtk_label_set_markup( (GtkLabel*)day_field, label_str );
                    Weekday_Abbr( data.week_day, label_str );
                    sprintf( label_str, "<span foreground=\"red\">%s</span>", label_str );
                    gtk_label_set_markup( (GtkLabel*)weekday_field, label_str );
                    sprintf( label_str, "<span foreground=\"red\">%02d</span>", data.month );
                    gtk_label_set_markup( (GtkLabel*)month_field, label_str );
                    sprintf( label_str, "<span foreground=\"red\">%02d</span>", data.year );
                    gtk_label_set_markup( (GtkLabel*)year_field, label_str );
                }
                break;

            case SYNC_AVAILABLE:
                gtk_label_set_text( (GtkLabel*)sync_field, "✓" );
                break;

            case THRESHOLD_AVAILABLE:
                sprintf( label_str, "%d", data.bit_threshold );
                gtk_label_set_text( (GtkLabel*)threshold_field, label_str );
                break;

            case UPDATE_PROGRESS:
                float progress_fraction = (float) data.bit_count / 58.0;
                gtk_progress_bar_set_fraction( (GtkProgressBar*)progress, progress_fraction );

                sprintf( label_str, "Bit % 2d/58", data.bit_count );
                gtk_label_set_text( (GtkLabel*)progress_field, label_str );
                break;

            case RECEPTION_CYCLE_END:
                gtk_label_set_text( (GtkLabel*)minute_field, "" );
                gtk_label_set_text( (GtkLabel*)hour_field, "" );
                gtk_label_set_text( (GtkLabel*)day_field, "" );
                gtk_label_set_text( (GtkLabel*)weekday_field, "" );
                gtk_label_set_text( (GtkLabel*)month_field, "" );
                gtk_label_set_text( (GtkLabel*)year_field, "" );
                gtk_label_set_text( (GtkLabel*)dst_field, "" );
                gtk_label_set_text( (GtkLabel*)dst_start_field, "" );
                gtk_label_set_text( (GtkLabel*)leap_second_field, "" );
                gtk_label_set_text( (GtkLabel*)sync_field, "⛌" );
                break;
        }

        event_received = false;
        pthread_cond_signal( &event_processed );
        pthread_mutex_unlock( &event_mutex );
    }

    pthread_mutex_destroy( &event_mutex );
    pthread_cond_destroy( &event_available );
    pthread_cond_destroy( &event_processed );

    return NULL;
} /* Gui_Handle_Event () */

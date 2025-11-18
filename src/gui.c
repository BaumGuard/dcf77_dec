#include "gui.h"

#include "decoder.h"
#include "common.h"
#include "event.h"

/*---------------------------------------------------------------*/

GtkWidget
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

/*---------------------------------------------------------------*/

// Callback functions

void on_start_button_clicked ( GtkButton* btn ) {
    if ( !start_button_pressed ) {
        start_button_pressed = true;

        decoder_running = true;

        pthread_create( &processing_thread, NULL, Start_Processing, NULL );
        pthread_create( &event_thread, NULL, Gui_Handle_Event, NULL );
        gtk_button_set_label( (GtkButton*)start_button, "Stop" );
    }
    else {
        start_button_pressed = false;

        gtk_progress_bar_set_fraction( (GtkProgressBar*)progress, 0.0 );
        gtk_label_set_text( (GtkLabel*)progress_field, "" );

        pthread_mutex_lock( &buf_mutex );

        pthread_cond_broadcast( &input_data_available );
        pthread_cond_broadcast( &envelope_data_available );
        pthread_cond_broadcast( &lpf_data_available );

        pthread_cond_broadcast( &event_available );
        pthread_cond_broadcast( &event_processed );

        pthread_mutex_unlock( &buf_mutex );

        gtk_button_set_label( (GtkButton*)start_button, "Start" );

        decoder_running = false;
    }
}

void on_program_close ( GtkWidget* widget, gpointer data ) {
    gtk_main_quit();
}

/*---------------------------------------------------------------*/

void* Launch_Gui ( void* ptr ) {
    GtkBuilder* builder;
    GtkWidget* window;

    gtk_init( NULL, NULL );

    builder = gtk_builder_new();
    gtk_builder_add_from_file( builder, "/usr/share/dcf77_dec/dcf77_dec.glade", NULL );

    window = GTK_WIDGET( gtk_builder_get_object(builder, "window") );
    gtk_window_set_title( (GtkWindow*)window, "DCF77 Decoder" );

    start_button = GTK_WIDGET( gtk_builder_get_object(builder, "start_button") );

    g_signal_connect( window, "destroy", G_CALLBACK(on_program_close), NULL );


    day_field = GTK_WIDGET( gtk_builder_get_object(builder, "day_field") );
    month_field = GTK_WIDGET( gtk_builder_get_object(builder, "month_field") );
    year_field = GTK_WIDGET( gtk_builder_get_object(builder, "year_field") );
    hour_field = GTK_WIDGET( gtk_builder_get_object(builder, "hour_field") );
    minute_field = GTK_WIDGET( gtk_builder_get_object(builder, "minute_field") );
    weekday_field = GTK_WIDGET( gtk_builder_get_object(builder, "weekday_field") );
    dst_field = GTK_WIDGET( gtk_builder_get_object(builder, "dst_field") );
    dst_start_field = GTK_WIDGET( gtk_builder_get_object(builder, "dst_start_field") );
    leap_second_field = GTK_WIDGET( gtk_builder_get_object(builder, "leap_second_field") );
    progress = GTK_WIDGET( gtk_builder_get_object(builder, "progress") );
    progress_field = GTK_WIDGET( gtk_builder_get_object(builder, "progress_field") );
    sample_rate_field = GTK_WIDGET( gtk_builder_get_object(builder, "sample_rate_field") );
    channels_field = GTK_WIDGET( gtk_builder_get_object(builder, "channels_field") );
    sample_type_field = GTK_WIDGET( gtk_builder_get_object(builder, "sample_type_field") );
    threshold_field = GTK_WIDGET( gtk_builder_get_object(builder, "threshold_field") );
    sync_field = GTK_WIDGET( gtk_builder_get_object(builder, "sync_field") );
    year_status_field = GTK_WIDGET( gtk_builder_get_object(builder, "year_status_field") );
    month_status_field = GTK_WIDGET( gtk_builder_get_object(builder, "month_status_field") );
    day_status_field = GTK_WIDGET( gtk_builder_get_object(builder, "day_status_field") );
    hour_status_field = GTK_WIDGET( gtk_builder_get_object(builder, "hour_status_field") );
    minute_status_field = GTK_WIDGET( gtk_builder_get_object(builder, "minute_status_field") );
    dayofweek_status_field = GTK_WIDGET( gtk_builder_get_object(builder, "dayofweek_status_field") );

    g_signal_connect( start_button, "clicked", G_CALLBACK(on_start_button_clicked), "TEST" );

    gtk_builder_connect_signals( builder, NULL );
    g_object_unref( builder );

    gtk_label_set_text( (GtkLabel*)channels_field, "1" );
    gtk_label_set_text( (GtkLabel*)sample_rate_field, "48000" );
    gtk_label_set_text( (GtkLabel*)sample_type_field, "S16_LE" );
    gtk_label_set_text( (GtkLabel*)sync_field, "⛌" );

    gtk_widget_show( window );
    gtk_main();

    return NULL;
}


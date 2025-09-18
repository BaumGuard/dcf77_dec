#include "ring_buffer.h"


void Init_Ring_Buffer ( Ring_Buffer* buf, uint32_t buffer_size ) {
    buf->buffer =
        (double*) malloc( buffer_size * sizeof(double) );
    buf->buffer_size = buffer_size;
    buf->head = 0;
    buf->tail = 0;
    buf->count = 0;

    pthread_mutex_init( &buf->mutex, NULL );
    pthread_cond_init( &buf->not_full, NULL );
    pthread_cond_init( &buf->not_empty, NULL );
} /* Init_Ring_Buffer() */

/*-----------------------------------------------------------*/

void Destroy_Buffer ( Ring_Buffer* buf ) {
    free( buf->buffer );
    pthread_mutex_destroy( &buf->mutex );
    pthread_cond_destroy( &buf->not_full );
    pthread_cond_destroy( &buf->not_empty );
} /* Destroy_Buffer() */

/*-----------------------------------------------------------*/

void Add_To_Buffer ( Ring_Buffer* buf, double element ) {

    pthread_mutex_lock( &buf->mutex );

    while ( buf->count == buf->buffer_size ) {
        pthread_cond_wait( &buf->not_full, &buf->mutex );
    }

    uint32_t index = buf->head % buf->buffer_size;
    buf->buffer[index] = element;
    buf->head++;
    buf->count++;

    pthread_cond_signal( &buf->not_empty );
    pthread_mutex_unlock( &buf->mutex );
} /* Add_To_Buffer() */

/*-----------------------------------------------------------*/

int Get_From_Buffer( Ring_Buffer* buf, double* element ) {
    pthread_mutex_lock( &buf->mutex );

    while ( buf->count == 0 ) {
        pthread_cond_wait( &buf->not_empty, &buf->mutex );
    }

    uint32_t index = buf->tail % buf->buffer_size;
    *element = buf->buffer[index];
    buf->tail++;
    buf->count--;

    pthread_cond_signal( &buf->not_full );
    pthread_mutex_unlock( &buf->mutex );

    return 1;
} /* Get_From_Buffer */

/*-----------------------------------------------------------*/

bool Buffer_Is_Empty ( Ring_Buffer* buf ) {
    return buf->count == 0;
} /* Buffer_Is_Empty() */

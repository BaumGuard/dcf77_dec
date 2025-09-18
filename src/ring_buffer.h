#pragma once

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

/*-----------------------------------------------------------*/

typedef struct {
    double* buffer;
    uint32_t buffer_size;
    uint32_t head;
    uint32_t tail;
    uint32_t count;

    pthread_mutex_t mutex;
    pthread_cond_t  not_full;
    pthread_cond_t  not_empty;
} Ring_Buffer;


/*-----------------------------------------------------------*/

void Init_Ring_Buffer ( Ring_Buffer* buf, uint32_t buffer_size );
void Destroy_Buffer ( Ring_Buffer* buf );

void Add_To_Buffer ( Ring_Buffer* buf, double element );
int Get_From_Buffer( Ring_Buffer* buf, double* element );

bool Buffer_Is_Empty ( Ring_Buffer* buf );

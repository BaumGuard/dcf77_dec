#include "decoder.h"

/*-----------------------------------------------------------*/

/*
Find the frequency of the DCF77 signal using DFT

Args:
 - buf : Ring buffer with the PCM values
*/
void Find_Signal_Frequency ( Ring_Buffer* buf ) {
    double
        real,
        imag,
        abs_value,
        max_fft_value = 0.0;


    int freq = 0;

    // Read first samples from buffer
    double* samples = (double*) malloc( sample_rate*sizeof(double) );

    int dft_size = sample_rate / 4;

    for ( int i=0; i<dft_size; i++ ) {
        Get_From_Buffer( buf, &samples[i] );
    }

    // DFT
    for ( int k=0; k<dft_size/2; k++ ) {

        real=0.0,
        imag=0.0;

        for ( int n=0; n<dft_size; n++ ) {
            real += samples[n] * cos( -(TWO_PI*(double)(k+1)*(double)n)
                / (double)sample_rate );
            imag += samples[n] * sin( -(TWO_PI*(double)(k+1)*(double)n)
                / (double)sample_rate );
        }

        abs_value = sqrt( real*real + imag*imag );

        // Find the frequency with the highest amplitude
        // This is assumed to be the frequency of the DCF77 signal
        if ( abs_value > max_fft_value ) {
            max_fft_value = abs_value;
            freq = k;
        }
    }
    signal_freq = freq;
    signal_freq_found = true;

    free( samples );
} /* Find_Signal_Frequency() */

/*-----------------------------------------------------------*/

/*
Goerzel's algorithm for a block of PCM values to find
the magnitude of a given frequency in the block

 - in_buffer  : PCM values
 - out_buffer : Frequency magnitudes for each block
*/
void* Goertzel ( void* arg ) {
    Thread_Data* thread_data = (Thread_Data*) arg;

    while ( !signal_freq_found ) {
        pthread_cond_wait( &goertzel_wait, &sample_reader_mutex );
    }

    double
        v0,
        v1,
        v2;

    double w = TWO_PI * signal_freq / sample_rate;

    double
        cosine = cos( w ),
        sine   = sin( w );

    double coeff = 2.0 * cosine;

    double pcm_value, goertzel_value;

    while ( true ) {
        v0 = 0.0;
        v1 = 0.0;
        v2 = 0.0;

        for ( uint32_t i = 0; i<GOERTZEL_BLOCK_SIZE; i++ ) {
            Get_From_Buffer( thread_data->in_buffer, &pcm_value );

            v2 = v1;
            v1 = v0;
            v0 = coeff * v1 - v2 + pcm_value;
        }

        double
            real = v1 - v2 * cosine,
            imag = v2 * sine,
            mag  = sqrt( real*real + imag*imag );

        goertzel_value = 2.0 * mag / GOERTZEL_BLOCK_SIZE;

        Add_To_Buffer( thread_data->out_buffer, goertzel_value );
    }

    return NULL;
} /* Goertzel() */


/*-----------------------------------------------------------*/

/*
Wait for the sync signal (1.8 s long carrier)

 - buf : Ring buffer with the Goertzel values
*/
void Wait_For_Sync ( Ring_Buffer* buf ) {
    uint32_t sync_goertzel_blocks =
        (uint32_t) (SYNC_DURATION /
        ( (double)GOERTZEL_BLOCK_SIZE / (double)sample_rate ) );

    int block_count = 0;
    double goertzel_value;

    while ( true ) {
        Get_From_Buffer( buf, &goertzel_value );

        if ( goertzel_value > threshold_bit_1 ) {
            while ( goertzel_value > threshold_bit_1 ) {
                block_count++;
                Get_From_Buffer( buf, &goertzel_value );
            }

            if ( Is_In_Range(block_count, sync_goertzel_blocks, BLOCK_TOLERANCE) ) {
                return;
            }

            block_count = 0;
        }
    }
} /* Wait_For_Sync() */

/*-----------------------------------------------------------*/

/*
Get the next bit from the signal based on the pause length

Pauses:
 - 100 ms -> Bit 0
 - 200 ms -> Bit 1

Args:
 - buf : Ring buffer with the Goertzel values
*/
uint8_t Get_Next_Bit ( Ring_Buffer* buf ) {
    uint32_t
        bit0_blocks =
            PAUSE_BIT_0 / ( (double)GOERTZEL_BLOCK_SIZE / (double)sample_rate ),
        bit1_blocks =
            PAUSE_BIT_1 / ( (double)GOERTZEL_BLOCK_SIZE / (double)sample_rate );

    int block_count = 0;

    double goertzel_value;

    while ( true ) {
        Get_From_Buffer( buf, &goertzel_value );

        if ( goertzel_value < threshold_bit_1 ) {
            while ( goertzel_value < threshold_bit_1 ) {
                block_count++;
                Get_From_Buffer( buf, &goertzel_value );
            }

            if ( Is_In_Range(block_count, bit0_blocks, BLOCK_TOLERANCE) ) {
                return 0;
            }
            else if ( Is_In_Range(block_count, bit1_blocks, BLOCK_TOLERANCE) ) {
                return 1;
            }

            block_count = 0;
        }
    }
} /* Get_Next_Bit() */


/*-----------------------------------------------------------*/

/*
Decode the DCF77 signal

Args:
 - in_buffer : Ring buffer with the Goertzel values
*/
void* Decode_DCF77 ( void* ptr ) {
    Thread_Data* thread_data = (Thread_Data*) ptr;

    pthread_mutex_lock( &sample_reader_mutex );

    fprintf( stderr, "Looking for signal frequency ...\n" );
    Find_Signal_Frequency( thread_data->opt_buffer );
    fprintf( stderr, "Found signal frequency : %d Hz\n", signal_freq );

    pthread_cond_signal( &goertzel_wait );
    pthread_mutex_unlock( &sample_reader_mutex );

    fprintf( stderr, "Calibrate signal level ...\n" );

    int blocks_per_second = sample_rate / GOERTZEL_BLOCK_SIZE;

    // Get the Goertzel values for one second to determine the
    // signal level threshold for bit 1
    double* goertzel_values =
        (double*) malloc( (blocks_per_second) * sizeof(double) );

    for ( int i=0; i<blocks_per_second; i++ ) {
        Get_From_Buffer( thread_data->in_buffer, &goertzel_values[i] );
    }

    // Sort the Goertzel values using the Bubble Sort algorithm
    int end = blocks_per_second - 1;
    double tmp;

    while ( end > 0 ) {
        for ( int j=0; j<end; j++ ) {
            if ( goertzel_values[j] > goertzel_values[j+1] ) {
                tmp = goertzel_values[j];
                goertzel_values[j] = goertzel_values[j+1];
                goertzel_values[j+1] = tmp;
            }
        }
        end--;
    }

    // Use the average of the lowest and highest signal level
    // as the threshold
    double avg = ( goertzel_values[0] + goertzel_values[blocks_per_second-1] ) / 2;
    threshold_bit_1 = avg + 0.15;

    printf("Threshold %f\n", threshold_bit_1);

    free( goertzel_values );

    uint8_t bits [60];

    while ( true ) {
        fprintf( stderr, "Waiting for sync ...\n" );
        Wait_For_Sync( thread_data->in_buffer );

        fprintf( stderr, "\nFound sync - Listening to signal ...\n" );

        for ( int i=0; i<59; i++ ) {
            bits[i] = Get_Next_Bit( thread_data->in_buffer );
            Progress_Bar( i+1, 59 );
        }

        DCF77_Data data;

        data.dst_starts = bits[16];

        if ( bits[17] == 1 && bits[18] == 0 ) {
            data.dst = 1;
        }
        else if ( bits[17] == 0 && bits[18] == 1 ) {
            data.dst = 0;
        }
        else {
            fprintf( stderr, "Corrupt signal - waiting for next sync ...\n" );
            continue;
        }

        data.leap_second_next_hour = bits[19];

        if ( bits[20] == 0 ) {
            fprintf( stderr, "Corrupt signal - waiting for next sync ...\n" );
            continue;
        }

        data.minute =
            1 * bits[21] +
            2 * bits[22] +
            4 * bits[23] +
            8 * bits[24] +
            10 * bits[25] +
            20 * bits[26] +
            40 * bits[27];

        uint8_t parity_minute = 0;
        for ( int i=21; i<=28; i++ ) {
            parity_minute += bits[i];
        }

        if ( parity_minute % 2 != 0 ) {
            fprintf( stderr, "Corrupt signal - waiting for next sync ...\n" );
            continue;
        }

        data.hour =
            1 * bits[29] +
            2 * bits[30] +
            4 * bits[31] +
            8 * bits[32] +
            10 * bits[33] +
            20 * bits[34];

        uint8_t parity_hour = 0;
        for ( int i=29; i<=35; i++ ) {
            parity_hour += bits[i];
        }

        if ( parity_hour % 2 != 0 ) {
            fprintf( stderr, "Corrupt signal - waiting for next sync ...\n" );
            continue;
        }

        data.day =
            1 * bits[36] +
            2 * bits[37] +
            4 * bits[38] +
            8 * bits[39] +
            10 * bits[40] +
            20 * bits[41];

        data.day_of_week =
            1 * bits[42] +
            2 * bits[43] +
            4 * bits[44];

        data.month =
            1 * bits[45] +
            2 * bits[46] +
            4 * bits[47] +
            8 * bits[48] +
            10 * bits[49];

        data.year =
            1 * bits[50] +
            2 * bits[51] +
            4 * bits[52] +
            8 * bits[53] +
            10 * bits[54] +
            20 * bits[55] +
            40 * bits[56] +
            80 * bits[57];

        uint8_t parity_date = 0;
        for ( int i=36; i<=58; i++ ) {
            parity_date += bits[i];
        }

        if ( parity_date % 2 != 0 ) {
            fprintf( stderr, "Corrupt signal - waiting for next sync ...\n" );
            continue;
        }

        Print_DCF77_Data( &data );
    }

    return NULL;
} /* Decode_DCF77() */


/*-----------------------------------------------------------*/

/*
Read the PCM values from stdin, convert them to double and
write them into a ring buffer

 - out_buffer : PCM values converted to double
*/
void* Read_Samples ( void* ptr ) {
    Thread_Data* thread_data = (Thread_Data*) ptr;

    // Determine the number of bytes per sample
    if (
        sample_data_type == S8 ||
        sample_data_type == U8
    ) {
        sample_size = 1;
    }
    else if (
        sample_data_type == S16_LE ||
        sample_data_type == U16_LE ||
        sample_data_type == S16_BE ||
        sample_data_type == U16_BE
    ) {
        sample_size = 2;
    }
    else if (
        sample_data_type == S24_LE ||
        sample_data_type == U24_LE ||
        sample_data_type == S24_BE ||
        sample_data_type == U24_BE
    ) {
        sample_size = 3;
    }
    else if (
        sample_data_type == S32_LE ||
        sample_data_type == U32_LE ||
        sample_data_type == S32_BE ||
        sample_data_type == U32_BE ||
        sample_data_type == F32
    ) {
        sample_size = 4;
    }
    else if (
        sample_data_type == F64
    ) {
        sample_size = 8;
    }


    // Determine whether we are working with Little Endian
    // or Big Endian
    if (
        sample_data_type == S16_LE ||
        sample_data_type == S24_LE ||
        sample_data_type == S32_LE ||
        sample_data_type == F32 ||
        sample_data_type == F64
    ) {
        is_little_endian = true;
    }
    else {
        is_little_endian = false;
    }

    //Sample sample;
    sample.i64 = 0;

    double sample_value;

    // Read, convert and write the samples to the ring buffer
    while ( true ) {

        pthread_mutex_lock( &capture_mutex );
        while ( data_available == 0 ) {
            pthread_cond_wait( &capture_wait, &capture_mutex );
        }


        switch ( sample_data_type ) {
            case S8:
                sample_value = (double) sample.i8 / (double) INT8_MAX;
                break;

            case U8:
                sample_value = (double) ( (int64_t)sample.u8 - INT8_MAX ) / (double) INT8_MAX;
                break;

            case S16_LE:
                sample_value = (double) sample.i16 / (double) INT16_MAX;
                break;

            case U16_LE:
                sample_value = (double) ( (int64_t)sample.u16 - INT16_MAX ) / (double) INT16_MAX;
                break;

            case S16_BE:
                sample_value = (double) sample.i16 / (double) INT16_MAX;
                break;

            case U16_BE:
                sample_value = (double) ( (int64_t)sample.u16 - INT16_MAX ) / (double) INT16_MAX;
                break;

            case S24_LE:
                sample_value = (double) sample.i24 / (double) INT24_MAX;
                break;

            case U24_LE:
                sample_value = (double) ( (int64_t)sample.u24 - INT24_MAX ) / (double) INT24_MAX;
                break;

            case S24_BE:
                sample_value = (double) sample.i24 / (double) INT24_MAX;
                break;

            case U24_BE:
                sample_value = (double) ( (int64_t)sample.u24 - INT24_MAX ) / (double) INT24_MAX;
                break;

            case S32_LE:
                sample_value = (double) sample.i32 / (double) INT32_MAX;
                break;

            case U32_LE:
                sample_value = (double) ( (int64_t)sample.u32 - INT32_MAX ) / (double) INT32_MAX;
                break;

            case S32_BE:
                sample_value = (double) sample.i32 / (double) INT32_MAX;
                break;

            case U32_BE:
                sample_value = (double) ( (int64_t)sample.u32 - INT32_MAX ) / (double) INT32_MAX;
                break;

            case F32:
                sample_value = (double) sample.f32;
                break;

            case F64:
                sample_value = sample.f64;
                break;
        }

        data_available = 0;

        pthread_cond_signal( &capture_wait );
        pthread_mutex_unlock( &capture_mutex );

        Add_To_Buffer( thread_data->out_buffer, sample_value );
    }

    return NULL;
} /* Read_Samples() */

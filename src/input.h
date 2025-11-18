#ifndef INPUT_H
#define INPUT_H

#define SAMPLE_RATE 48000

/*
Read audio data from the ALSA interface "pulse" and
write it into the ring buffer

 - Sample rate  : 48000 Hz
 - Bit depth    : 16 bit
 - Data type    : S16_LE
 - Channels     : 1
*/
void* Alsa_Capture ( void* arg );

#endif

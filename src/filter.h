#ifndef FILTER_H
#define FILTER_H

/*
Low pass filter (second degree IIR Butterworth filter)

This filter is applied to the audio data from the envelope
detector
*/
void* LPF ( void* arg );

#endif

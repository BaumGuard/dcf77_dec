#ifndef DECODER_H
#define DECODER_H

#define SYNC_DURATION 1.8
#define BIT0_DURATION 0.1
#define BIT1_DURATION 0.2
#define TOLERANCE 300

/*
Get the bit sequence and decode the information
Report to the event handler whenever a new value is available
*/
void* Start_Processing ( void* arg );

#endif

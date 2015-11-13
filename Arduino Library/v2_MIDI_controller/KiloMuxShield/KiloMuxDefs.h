/*
KiloMux Library
 */
#ifndef KiloMuxDefs_h
#define KiloMuxDefs_h

#include "Arduino.h"
#include "NewPing.h"
#include <WProgram.h>

#define NUM_MUX               2            // Number of multiplexers to address
#define NUM_MUX_CHANNELS      16           // Number of multiplexing channels

#define NUM_595               2            // Número de integrados 74HC595          - Si usas el KiloMux Shield, poner "2"
#define NUM_SR_OUTPUTS        NUM_595*8    // Número de LEDs en total, NO CAMBIAR

// Note On and Note Off values
#define NOTE_ON   127
#define NOTE_OFF  0

#define ON  1
#define OFF 0

// For analog threshold filtering
#define ANALOG_UP   1      // DO NOT CHANGE
#define ANALOG_DOWN 0      // DO NOT CHANGE
#define NOISE_THR   1      // If you change this, you'll skip more values when a pot or sensor changes direction

// Prescalers para el ADC /////////////////////////////////////////////////////////////////
const unsigned char PS_16 = (1 << ADPS2);
const unsigned char PS_32 = (1 << ADPS2) | (1 << ADPS0);
const unsigned char PS_64 = (1 << ADPS2) | (1 << ADPS1);
const unsigned char PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
///////////////////////////////////////////////////////////////////////////////////////////

#endif

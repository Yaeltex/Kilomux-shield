/*
 * Author: Franco Grassano - YAELTEX
 * Date: 18/02/2016
 * Buenos Aires, Argentina - 2015
 * ---
 * LICENSE INFO
 * Kilomux Shield library for Arduino, by Yaeltex, is released by
 * Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0) - http://creativecommons.org/licenses/by-sa/4.0/
 * ----
 *
 * Defines, variables and constants used in the Kilomux Shield Library
 *
 */
 
#ifndef KilomuxDefs_h // Imports only if not imported earlier
#define KilomuxDefs_h

#include "Arduino.h"

#define MUX_A                0            // Mux A identifier
#define MUX_B                1            // Mux B identifier

#define MUX_A_PIN            A0           // Mux A pin
#define MUX_B_PIN            A1           // Mux B pin

#define MUX_A1_START         0            // Mux A1 header first pin
#define MUX_A1_END           7            // Mux A1 header last pin
#define MUX_A2_START         8            // Mux A2 header first pin
#define MUX_A2_END           15           // Mux A2 header last pin
#define MUX_B1_START         0            // Mux B1 header first pin
#define MUX_B1_END           7            // Mux B1 header last pin
#define MUX_B2_START         8            // Mux B2 header first pin
#define MUX_B2_END           15           // Mux B2 header last pin

#define NUM_MUX              2            // Number of multiplexers to address
#define NUM_MUX_CHANNELS     16           // Number of multiplexing channels

#define NUM_595              2            // Number of 74HC595 ICs
#define NUM_OUTPUTS       	 NUM_595*8    // Total number of shift register outputs

// Note On and Note Off values
#define NOTE_ON   127
#define NOTE_OFF  0

// On and Off labels
#define ON  1
#define OFF 0

// Pull up mode for digital inputs
#define PULLUP      1

// For analog threshold filtering
#define ANALOG_UP   1      // DO NOT CHANGE
#define ANALOG_DOWN 0      // DO NOT CHANGE
#define NOISE_THR   1      // If you change this, you'll skip more values when a pot or sensor changes direction

// Prescalers for ADC /////////////////////////////////////////////////////////////////
const unsigned char PS_16 = (1 << ADPS2);                                 // PS_16 (1MHz - 50000 samples/s)
const unsigned char PS_32 = (1 << ADPS2) | (1 << ADPS0);                  // PS_32 (500KHz - 31250 samples/s)
const unsigned char PS_64 = (1 << ADPS2) | (1 << ADPS1);                  // PS_64 (250KHz - 16666 samples/s)
const unsigned char PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);  // PS_128 (125KHz - 8620 samples/s)
///////////////////////////////////////////////////////////////////////////////////////////

// Ultrasonic Sensor lines
const int ActivateButtonPin = 10;
const int ActivatedLedPin = 11;
const int EchoPin = 12;
const int TriggerPin = 13;

#endif

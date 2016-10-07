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
 * Header file, with class definition
 *
 */

#ifndef Kilomux_h     // Imports header file only if not imported earlier
#define Kilomux_h

#include "KilomuxDefs.h"

class Kilomux
{
public:                             // These are methods and variables accesible from Arduino IDE
    Kilomux();                      // Class constructor
    // Methods
	void init(void);																	  // Initialize everything to use Kilomux Shield
    void digitalWriteKm(int out, int state);                                              // Write digital data to shift register outputs
	void digitalWritePortKm(byte portState, int port);                                    // Write digital data of an entire port and update shift register outputs
	void digitalWritePortsKm(byte portState1, byte portState2);                           // Write digital data of both ports and update shift register outputs
    int digitalReadKm(int mux, int chan);                                                 // Read digital data from multiplexer
    int digitalReadKm(int mux, int chan, int pullup);                                     // Read digital data from multiplexer, setting an internal pullup resistor
    int analogReadKm(int mux, int chan);                                                  // Read analog data from multiplexer
    void setADCprescaler(byte setting);                                                   // Set Analog to Digital Converter (ADC) prescaler to change number of readings per second
    unsigned int isNoise(unsigned int newValue, unsigned int prevValue, bool direction);  // Method to determine if the reading is noise or a true change

    // Mux readings
    unsigned int muxReadings[NUM_MUX][NUM_MUX_CHANNELS];                  // Present readings
    unsigned int muxPrevReadings[NUM_MUX][NUM_MUX_CHANNELS];              // Previous readings

private:
    // Methods
    void clearRegisters595();                       // Set every output to LOW
    void writeRegisters595();                       // Write the chip registers

    // Variables
    // Address lines for multiplexer
  	const int _S0 = 2;
    const int _S1 = 3;
    const int _S2 = 4;
    const int _S3 = 5;
  	// Input signal of multiplexers
    const int InMuxA = A0;
    const int InMuxB = A1;

  	// Shift register lines
  	const int DataPin = 6;                           // Data Pin (DS) connected to pin 14 of 74HC595
  	const int LatchPin = 7;                          // Latch Pin (ST_CP) connected to pin 12 of 74HC595
  	const int ClockPin = 8;                          // Clock Pin (SH_CP) connected to pin 11 of 74HC595

    bool outputState[NUM_OUTPUTS];                // State of every output

    // Do not change - These are used to have the inputs and outputs of the headers in order
    byte MuxAMapping[NUM_MUX_CHANNELS] =  {0,        // INPUT 0   - Mux channel 0
                                           7,        // INPUT 1   - Mux channel 7
                                           1,        // INPUT 2   - Mux channel 1
                                           6,        // INPUT 3   - Mux channel 6
                                           2,        // INPUT 4   - Mux channel 2
                                           5,        // INPUT 5   - Mux channel 5
                                           3,        // INPUT 6   - Mux channel 3
                                           4,        // INPUT 7   - Mux channel 4
                                           15,       // INPUT 8   - Mux channel 15
                                           11,       // INPUT 9   - Mux channel 11
                                           14,       // INPUT 10  - Mux channel 14
                                           10,       // INPUT 11  - Mux channel 10
                                           13,       // INPUT 12  - Mux channel 13
                                           9,        // INPUT 13  - Mux channel 9
                                           12,       // INPUT 14  - Mux channel 12
                                           8};       // INPUT 15  - Mux channel 8

    byte MuxBMapping[NUM_MUX_CHANNELS] =   {0,       // INPUT 0   - Mux channel 0
                                            7,       // INPUT 1   - Mux channel 7
                                            1,       // INPUT 2   - Mux channel 1
                                            6,       // INPUT 3   - Mux channel 6
                                            2,       // INPUT 4   - Mux channel 2
                                            5,       // INPUT 5   - Mux channel 5
                                            3,       // INPUT 6   - Mux channel 3
                                            4,       // INPUT 7   - Mux channel 4
                                            15,      // INPUT 8   - Mux channel 15
                                            11,      // INPUT 9   - Mux channel 11
                                            14,      // INPUT 10  - Mux channel 14
                                            10,      // INPUT 11  - Mux channel 10
                                            13,      // INPUT 12  - Mux channel 13
                                            9,       // INPUT 13  - Mux channel 9
                                            12,      // INPUT 14  - Mux channel 12
                                            8};      // INPUT 15  - Mux channel 8

    byte OutputMapping[NUM_MUX_CHANNELS] =  {7,      // OUTPUT 0  - Shift register output 7
                                             0,      // OUTPUT 1  - Shift register output 0
                                             6,      // OUTPUT 2  - Shift register output 6
                                             3,      // OUTPUT 3  - Shift register output 3
                                             5,      // OUTPUT 4  - Shift register output 5
                                             2,      // OUTPUT 5  - Shift register output 2
                                             4,      // OUTPUT 6  - Shift register output 4
                                             1,      // OUTPUT 7  - Shift register output 1
                                             15,     // OUTPUT 8  - Shift register output 15
                                             8,      // OUTPUT 9  - Shift register output 8
                                             14,     // OUTPUT 10 - Shift register output 14
                                             11,     // OUTPUT 11 - Shift register output 11
                                             13,     // OUTPUT 12 - Shift register output 13
                                             10,     // OUTPUT 13 - Shift register output 10
                                             12,     // OUTPUT 14 - Shift register output 12
                                             9};     // OUTPUT 15 - Shift register output 9
};

#endif

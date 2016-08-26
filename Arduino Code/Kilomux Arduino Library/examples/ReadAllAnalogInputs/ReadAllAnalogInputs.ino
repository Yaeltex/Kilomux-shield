
/*
 * Author: Franco Grassano - YAELTEX
 * Date: 18/02/2016
 * ---
 * LICENSE INFO
 * Kilomux Shield by Yaeltex is released by
 * * Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0) - http://creativecommons.org/licenses/by-sa/4.0/
 * ----
 * Description: Reads several analog inputs for any of the shield's ports, and displays them on the serial monitor.
 *              Change MUX_START and MUX_END to read other inputs.
 *              To read MUX_B, change parameter in analogReadKM function.
 *              To read digital sensors, like buttons, instead of analog,change function to digitalReadKM.
 *              This example is for use with the Kilomux Shield.
 * 
 * Kilomux Library is available at https://github.com/Yaeltex/Kilomux-Shield/blob/master/Arduino%20Code/KilomuxShield%20Library/KilomuxShield.zip
 */

#include <Kilomux.h>          // Import class declaration
#include <KilomuxDefs.h>      // Import KiloMux defines
#include <MIDI.h>             // Import Arduino MIDI Library headers
#include <midi_Defs.h>
#include <midi_Message.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>

MIDI_CREATE_DEFAULT_INSTANCE() // Create a hardware instance of the library

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define SERIAL_COMMS          // Leave uncommented (and comment the line below) to send over serial
//#define MIDI_COMMS          // Leave uncommented (and comment the line above) to send midi messages
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define NUM_OF_INPUTS_TO_READ   8        // We'll read the 8 inputs of por A1.

#define MIDI_CHANNEL            1        // Pots values will be sent over channel 1 over MIDI port or USB

#define ANALOG_INCREASING   0       
#define ANALOG_DECREASING   1

#define NOISE_THRESHOLD     3

// Array to store CC values that we want to use. We need to add or remove lines if we change NUM_OF_INPUTS_TO_READ
unsigned int CCmap[NUM_OF_INPUTS_TO_READ] = { 7,    // CC to send when input 0 changes
                                              6,    // CC to send when input 1 changes
                                              5,    // CC to send when input 2 changes
                                              4,    // CC to send when input 3 changes
                                              3,    // CC to send when input 4 changes
                                              2,    // CC to send when input 5 changes
                                              1,    // CC to send when input 6 changes
                                              0};   // CC to send when input 7 changes

Kilomux KmShield;             // Kilomux Shield  

unsigned int analogData[NUM_OF_INPUTS_TO_READ];         // Variable to store analog values
unsigned int analogDataPrev[NUM_OF_INPUTS_TO_READ];     // Variable to store previous analog values
byte analogDirection[NUM_OF_INPUTS_TO_READ];            // "static" means the value is stored and can be used next time this function runs

void setup() {
  KmShield.init();                                    // Initialize Kilomux shield hardware
  
  // Set all elements in arrays to 0
  for(int i = 0; i < NUM_OF_INPUTS_TO_READ; i++){
     analogData[i] = 0;
     analogDataPrev[i] = 0;
     analogDirection[i] = 0;
  }
  
  #if defined(SERIAL_COMMS)
  Serial.begin(115200);       // Initialize serial
  #elif defined(MIDI_COMMS)
  MIDI.begin(MIDI_CHANNEL_OMNI); MIDI.turnThruOff();  // Initialize MIDI port. Turn MIDI-thru off, which is on by default
  #endif
}

void loop() {  
  for (int input = MUX_A1_START; input <= MUX_A1_END; input++){      // Sweeps all 8 multiplexer inputs of Mux A1 header
    analogData[input] = KmShield.analogReadKm(MUX_A, input);           // Read analog value from MUX_A and channel 'input'
    
    if(!IsNoise(input)){
      #if defined(SERIAL_COMMS)
      Serial.print("IN "); Serial.print(CCmap[input]);                        // Print all values in a format like
      Serial.print(": ");                                                     //    IN 0: 581  IN 1: 230 ...... IN 8: 500 
      Serial.print(analogData[input]>>3);                                     //    IN 0: 582  IN 1: 230 ...... IN 8: 500
      Serial.print("\t");                                                     //    IN 0: 583  IN 1: 230 ...... IN 8: 500
      #elif defined(MIDI_COMMS)
      MIDI.sendControlChange(CCmap[input], analogData[input]>>3, MIDI_CHANNEL);  // Or send midi message. 
                                                                                 // analogData >> 3, divides by 2^3 = 8, the value, to adjust to midi resolution (1024/8 = 128)
      #endif  
    }
  }                                                                  
  #if defined(SERIAL_COMMS)
  Serial.println("");                                                // New Line
  #endif
}

// Thanks to Pablo Fullana for the help with this function!
// It's just a threshold filter. If the new value stays within the previous value + - the noise threshold set, then it's considered noise
unsigned int IsNoise(unsigned int input) {
  if (analogDirection[input] == ANALOG_INCREASING){   // CASE 1: If signal is increasing,
    if(analogData[input] > analogDataPrev[input]){      // and the new value is greater than the previous,
       analogDataPrev[input] = analogData[input];       // store new value as previous and return
       return 0;                                        // NOT NOISE!
    }
    else if(analogData[input] < analogDataPrev[input] - NOISE_THRESHOLD){  // If, otherwise, it's lower than the previous value and the noise threshold together,
      analogDirection[input] = ANALOG_DECREASING;                           // means it started to decrease,
      analogDataPrev[input] = analogData[input];                            // so store new value as previous and return    
      return 0;                                                             // NOT NOISE!
    }
  }
  if (analogDirection[input] == ANALOG_DECREASING){   // CASE 2: If signal is increasing,                         
    if(analogData[input] < analogDataPrev[input]){      // and the new value is lower than the previous,
       analogDataPrev[input] = analogData[input];       // store new value as previous and return
       return 0;                                        // NOT NOISE!
    }
    else if(analogData[input] > analogDataPrev[input] + NOISE_THRESHOLD){  // If, otherwise, it's greater than the previous value and the noise threshold together,
      analogDirection[input] = ANALOG_INCREASING;                            // means it started to increase,
      analogDataPrev[input] = analogData[input];                             // so store new value as previous and return
      return 0;                                                              // NOT NOISE!
    }
  }
  return 1;                                           // If everyting above was not true, then IT'S NOISE! Pot is trying to fool us. But we owned you pot ;)
}

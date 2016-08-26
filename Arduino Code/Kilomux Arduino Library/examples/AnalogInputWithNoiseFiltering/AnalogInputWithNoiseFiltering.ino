/*
 * Author: Franco Grassano - YAELTEX
 * Date: 18/02/2016
 * ---
 * LICENSE INFO
 * Kilomux Shield by Yaeltex is released by
 * Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0) - http://creativecommons.org/licenses/by-sa/4.0/
 * ----
 * Description: Analog input reading with threshold filtering to avoid flicker readings.
 *              Sends readings via Serial or MIDI. Choose which, by commenting one of the lines, where it is indicated.
 *              This example is for use with the KiloMux Shield.
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

#define MIDI_CHANNEL  1        // Pot value will be sent with CC#10 (Pan) over channel 1 over MIDI port or USB
#define POT_CC        10        

#define ANALOG_INCREASING   0       
#define ANALOG_DECREASING   1

#define NOISE_THRESHOLD     3 

Kilomux KmShield;             // Kilomux Shield instance
unsigned int potInput = 0;    // Shield input where we connected a potentiometer or a sensor (0-15)

void setup() {
  KmShield.init();                                    // Initialize Kilomux shield hardware
  
  //Serial.begin(115200);                               // Initialize serial                                                  <--|      
                                                                                                                            // |-> Only one of these lines should be uncommented. Choose MIDI or Serial
  MIDI.begin(MIDI_CHANNEL_OMNI); MIDI.turnThruOff();  // Initialize MIDI port. Turn MIDI-thru off, which is on by default   <--|
}

void loop() {
  int analogData = 0;                                           // Variable to store analog values
  
  analogData = KmShield.analogReadKm(MUX_A, potInput);          // Read analog value from MUX_A and channel 'potInput' (0-15) 
  
  if (!IsNoise(analogData)){                                     
    //Serial.print("Pot reading: "); Serial.println(analogData);    // If IsNoise() returns 0 , the value is not noise. Print value to serial monitor   <--| 
                                                                                                                                                      // |-> Only one of these lines should be uncommented. Choose MIDI or Serial
    MIDI.sendControlChange(POT_CC, analogData>>3, MIDI_CHANNEL);  // Or send midi message.                                                            <--|
                                                                  // analogData >> 3, divides by 2^3 = 8, the value, to adjust to midi resolution (1024/8 = 128)
  }
  else{                                                           // If IsNoise() returns 1, then the new analog value is just noise
                                                                  // in which case, do nothing
  }
  
}

// Thanks to Pablo Fullana for the help with this function!
// It's just a threshold filter. If the new value stays within the previous value + - the noise threshold set, then it's considered noise
unsigned int IsNoise(int newValue) {
  static bool analogDirection = 0;            // "static" means the value is stored and can be used next time this function runs
  static unsigned int prevValue = 0;          // Store previous analog value
  
  if (analogDirection == ANALOG_INCREASING){          // CASE 1: If signal is increasing,
    if(newValue > prevValue){                           // and the new value is greater than the previous,
       prevValue = newValue;                            // store new value as previous and return
       return 0;                                        // NOT NOISE!
    }
    else if(newValue < prevValue - NOISE_THRESHOLD){  // If, otherwise, it's lower than the previous value and the noise threshold together,
      analogDirection = ANALOG_DECREASING;              // means it started to decrease,
      prevValue = newValue;                             // so store new value as previous and return    
      return 0;                                         // NOT NOISE!
    }
  }
  if (analogDirection == ANALOG_DECREASING){          // CASE 2: If signal is increasing,                         
    if(newValue < prevValue){                           // and the new value is lower than the previous,
       prevValue = newValue;                            // store new value as previous and return
       return 0;                                        // NOT NOISE!
    }
    else if(newValue > prevValue + NOISE_THRESHOLD){  // If, otherwise, it's greater than the previous value and the noise threshold together,
      analogDirection = ANALOG_INCREASING;              // means it started to increase,
      prevValue = newValue;                             // so store new value as previous and return
      return 0;                                         // NOT NOISE!
    }
  }
  return 1;                                           // If everyting above was not true, then IT'S NOISE! Pot is trying to fool us. But we owned you pot ;)
}

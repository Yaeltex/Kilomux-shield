/*
 * Author: Franco Grassano - YAELTEX
 * Date: 18/02/2016
 * ---
 * LICENSE INFO
 * Kilomux Shield by Yaeltex is released by
 * * Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0) - http://creativecommons.org/licenses/by-sa/4.0/
 * ----
 * Description: Reads one analog input and sends value over serial or MIDI.
 *              This example is for use with the Kilomux Shield.
 * 
 * Kilomux Library is available at https://github.com/Yaeltex/Kilomux-Shield/blob/master/Arduino%20Code/KilomuxShield%20Library/KilomuxShield.zip
 */
 
#include <Kilomux.h>              // Import class declaration
#include <KilomuxDefs.h>          // Import Kilomux defines
#include <MIDI.h>                 // Import Arduino MIDI Library headers
#include <midi_Defs.h>
#include <midi_Message.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>

MIDI_CREATE_DEFAULT_INSTANCE() // Create a hardware instance of the library

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define SERIAL_COMMS          // Leave uncommented (and comment the line below) to send over serial
//#define MIDI_COMMS          // Leave uncommented (and comment the line above) to send midi messages
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MIDI_CHANNEL  1        // Pot value will be sent with CC#74 over channel 1 over MIDI port or USB
#define POT_CC        74       // CC#74 -> VCF's cutoff freq

Kilomux KmShield;              // Kilomux Shield    

unsigned int potInput = 0;     // Shield input where we connected a potentiometer or a sensor (0-15)

void setup() { 
  KmShield.init();                                    // Initialize Kilomux shield hardware
  
  #if defined(SERIAL_COMMS)
  Serial.begin(115200);                               // Initialize serial
  #elif defined(MIDI_COMMS)
  MIDI.begin(MIDI_CHANNEL_OMNI); MIDI.turnThruOff();  // Initialize MIDI port. Turn MIDI-thru off, which is on by default
  #endif
}

void loop() {
  int analogData = 0;                                           // Variable to store analog values
  analogData = KmShield.analogReadKm(MUX_A, potInput);          // Read analog value from MUX_A and channel 'potInput' (0-15) 
  
  #if defined(SERIAL_COMMS)
  Serial.print("Pot reading: "); Serial.println(analogData);    // Print value at the serial monitor                        
  #elif defined(MIDI_COMMS)                                                                                                 
  MIDI.sendControlChange(POT_CC, analogData>>3, MIDI_CHANNEL);  // Or send midi message.                                 
  #endif                                                        // analogData >> 3, divides by 2^3 = 8, the value, to adjust to midi resolution (1024/8 = 128)                                                                 
}

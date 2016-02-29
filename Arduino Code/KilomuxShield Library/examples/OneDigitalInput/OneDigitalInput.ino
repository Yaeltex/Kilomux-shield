/*
 * Author: Franco Grassano - YAELTEX
 * Date: 18/02/2016
 * ---
 * LICENSE INFO
 * Kilomux Shield by Yaeltex is released by
 * * Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0) - http://creativecommons.org/licenses/by-sa/4.0/
 * ----
 * Description: Reads button with external PullUp resistor, and sends its state over serial.
 *              This example is for use with the Kilomux Shield.
 * 
 * Kilomux Library is available at https://github.com/Yaeltex/KiloMux-Shield/blob/master/Arduino%20Code/KiloMuxShield%20Library/KiloMuxShield.zip
 */
 
#include <Kilomux.h>              // Import class declaration
#include <KilomuxDefs.h>          // Import Kilomux defines

Kilomux KmShield;                 // Kilomux Shield  

unsigned int buttonInput = 1;     // Shield input where we connected a button or a sensor (1-16)
                                  // Remember to add a 10K pull-up resistor (from the button input to 5V)

void setup() {
  Serial.begin(115200);           // Initialize serial at 115200bps
}

void loop() {
  int buttonState = 0;                                          // Variable to store digital values
  buttonState = KmShield.digitalReadKm(MUX_A, buttonInput);     // Read digital value from MUX_A and channel 'buttonInput' (1-16)   
  
  Serial.print("Button state: "); Serial.println(buttonState);  // print value at the serial monitor
}


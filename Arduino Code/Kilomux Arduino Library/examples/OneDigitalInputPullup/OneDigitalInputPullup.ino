/*
 * Author: Franco Grassano - YAELTEX
 * Date: 18/02/2016
 * ---
 * LICENSE INFO
 * Kilomux Shield by Yaeltex is released by
 * * Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0) - http://creativecommons.org/licenses/by-sa/4.0/
 * ----
 * Description: Reads button setting internal PullUp resistor, and sends its state over serial.
 *              This example is for use with the Kilomux Shield.
 * 
 * Kilomux Library is available at https://github.com/Yaeltex/Kilomux-Shield/blob/master/Arduino%20Code/KilomuxShield%20Library/KilomuxShield.zip
 */
 
#include <Kilomux.h>              // Import class declaration
#include <KilomuxDefs.h>          // Import Kilomux defines

Kilomux KmShield;                 // Kilomux Shield  

unsigned int buttonInput = 9;     // Shield input where we connected a button or a sensor (1-16)
                                  // No need for 10K pull-up resistor in this example

void setup() {
  Serial.begin(115200);           // Initialize serial at 115200bps
}

void loop() {
  int buttonState = 0;                                                  // Variable to store digital values
  buttonState = KmShield.digitalReadKm(MUX_B, buttonInput, PULLUP);     // Read digital value from MUX_A and channel 'buttonInput' (1-16), and set internal pullup  
  
  Serial.print("Button state: "); Serial.println(buttonState);          // print value at the serial monitor
}


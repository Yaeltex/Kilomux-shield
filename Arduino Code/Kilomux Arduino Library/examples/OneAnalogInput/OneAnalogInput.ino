/*
 * Author: Franco Grassano - YAELTEX
 * Date: 18/02/2016
 * ---
 * LICENSE INFO
 * Kilomux Shield by Yaeltex is released by
 * * Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0) - http://creativecommons.org/licenses/by-sa/4.0/
 * ----
 * Description: Reads one analog input and sends value over serial.
 *              This example is for use with the Kilomux Shield.
 * 
 * Kilomux Library is available at https://github.com/Yaeltex/Kilomux-Shield/blob/master/Arduino%20Code/KilomuxShield%20Library/KilomuxShield.zip
 */
 
#include <Kilomux.h>          // Import class declaration
#include <KilomuxDefs.h>

Kilomux KmShield;             // Kilomux Shield    

unsigned int potInput = 1;    // Shield input where we connected a potentiometer or a sensor (1-16)

void setup() {
  Serial.begin(115200);       // Initialize serial
}

void loop() {
  int analogData = 0;                                           // Variable to store analog values
  analogData = KmShield.analogReadKm(MUX_A, potInput);          // Read analog value from MUX_A and channel 'potInput' (1-16) 
  
  Serial.print("Pot reading: "); Serial.println(analogData);    // Print value at the serial monitor
}



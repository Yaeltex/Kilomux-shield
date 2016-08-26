/*
 * Author: Franco Grassano - YAELTEX
 * Date: 18/02/2016
 * ---
 * LICENSE INFO
 * Kilomux Shield by Yaeltex is released by
 * Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0) - http://creativecommons.org/licenses/by-sa/4.0/
 * ----
 * Description: Blink an LED.
 *              This example is for use with the Kilomux Shield.
 * 
 * Kilomux Library is available at https://github.com/Yaeltex/Kilomux-Shield/blob/master/Arduino%20Code/KilomuxShield%20Library/KilomuxShield.zip
 */
 
#include <Kilomux.h>          // Import class declaration
#include <KilomuxDefs.h>      // Import Kilomux defines

Kilomux KmShield;             // KiloMux Shield   

#define MS_ON  1000             // Number of milliseconds the LED will be on 
#define MS_OFF 1000             // Number of milliseconds the LED will be off

unsigned int ledOutput = 0;   // KiloMux output where we already connected an LED (0-15)

void setup() {
  KmShield.init();
}

void loop() {
  KmShield.digitalWriteKm(ledOutput, HIGH);     // Turn led on
  delay(MS_ON);                                 // Wait. LED is on.
  KmShield.digitalWriteKm(ledOutput, LOW);      // Now turn it off. 
  delay(MS_OFF);                                // Wait. It's off.
}


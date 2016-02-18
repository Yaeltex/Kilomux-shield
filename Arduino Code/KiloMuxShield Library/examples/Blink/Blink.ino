/*
 * Author: Franco Grassano - YAELTEX
 * Date: 18/02/2016
 * ---
 * LICENSE INFO
 * Kilo Mux Shield by Yaeltex is released by
 * Creative Commons Atribución-CompartirIgual 4.0 Internacional - http://creativecommons.org/licenses/by-sa/4.0/
 * ----
 * Description: Blink an LED.
 *              This example is for use with the KiloMux Shield.
 * 
 * KiloMux Library is available at https://github.com/Yaeltex/KiloMux-Shield/blob/master/Arduino%20Code/KiloMuxShield%20Library/KiloMuxShield.zip
 */
 
#include <KiloMux.h>          // Import class declaration
#include <KiloMuxDefs.h>      // Import KiloMux defines

KiloMux KMShield;             // KiloMux Shield   

#define MS_ON  1000             // Number of milliseconds the LED will be on 
#define MS_OFF 1000             // Number of milliseconds the LED will be off

unsigned int ledOutput = 1;   // KiloMux output where we already connected an LED (1-16)

void setup() {
  // Nothing here  
}

void loop() {
  KMShield.digitalWriteKM(ledOutput, HIGH);     // Turn led on
  delay(MS_ON);                                 // Wait. LED is on.
  KMShield.digitalWriteKM(ledOutput, LOW);      // Now turn it off. 
  delay(MS_OFF);                                // Wait. It's off.
}


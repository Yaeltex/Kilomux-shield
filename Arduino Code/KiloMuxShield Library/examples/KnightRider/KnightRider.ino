/*
 * Author: Franco Grassano - YAELTEX
 * Date: 18/02/2016
 * ---
 * LICENSE INFO
 * Kilo Mux Shield by Yaeltex is released by
 * Creative Commons Atribución-CompartirIgual 4.0 Internacional - http://creativecommons.org/licenses/by-sa/4.0/
 * ----
 * Description: Performs Knight Rider over shield's 16 outputs.
 *              This example is for use with the KiloMux Shield.
 * 
 * KiloMux Library is available at https://github.com/Yaeltex/KiloMux-Shield/blob/master/Arduino%20Code/KiloMuxShield%20Library/KiloMuxShield.zip
 */
 
#include <KiloMux.h>          // Import class declaration
#include <KiloMuxDefs.h>      // Import KiloMux defines

KiloMux KMShield;             // KiloMux Shield   

#define MS_ON  40             // Number of milliseconds the LED will be on 
#define MS_OFF 4              // Number of milliseconds the LED will be off

void setup() {
  // Nothing here
}

void loop() {
  for (int led = 0; led < NUM_OUTPUTS; led++){        // Sweeps all 16 outputs
    KMShield.digitalWriteKM(led, HIGH);               // Turn led on
    delay(MS_ON);                                     // Wait. LED is on.
    KMShield.digitalWriteKM(led, LOW);                // Now turn it off. 
    delay(MS_OFF);                                    // Wait. It's off.
  } 
}


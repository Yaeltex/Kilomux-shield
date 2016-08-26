/*
 * Author: Franco Grassano - YAELTEX
 * Date: 18/02/2016
 * ---
 * LICENSE INFO
 * Kilomux Shield by Yaeltex is released by
 * * Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0) - http://creativecommons.org/licenses/by-sa/4.0/
 * ----
 * Description: Performs Knight Rider over shield's 16 outputs.
 *              This example is for use with the Kilomux Shield.
 * 
 * Kilomux Library is available at https://github.com/Yaeltex/Kilomux-Shield/blob/master/Arduino%20Code/KilomuxShield%20Library/KilomuxShield.zip
 */
 
#include <Kilomux.h>          // Import class declaration
#include <KilomuxDefs.h>      // Import KiloMux defines

Kilomux KmShield;             // Kilomux Shield   

#define MS_ON  40             // Number of milliseconds the LED will be on 
#define MS_OFF 4              // Number of milliseconds the LED will be off

void setup() {
  KmShield.init();
}

void loop() {
  for (int led = 0; led < NUM_OUTPUTS; led++){        // Sweeps all 16 outputs
    KmShield.digitalWriteKm(led, HIGH);               // Turn led on
    delay(MS_ON);                                     // Wait. LED is on.
    KmShield.digitalWriteKm(led, LOW);                // Now turn it off. 
    delay(MS_OFF);                                    // Wait. It's off.
  } 
}


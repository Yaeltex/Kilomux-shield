#include <KiloMux.h>          // Import class declaration
#include <KiloMuxDefs.h>      // Import KiloMux defines

KiloMux KMShield;             // KiloMux Shield   

#define MS_ON  40             // Number of milliseconds the LED will be on 
#define MS_OFF 4              // Number of milliseconds the LED will be off

void setup() {
  // Nothing here
}

void loop() {
  for (int led = 0; led < NUM_SR_OUTPUTS; led++){   // Sweeps all 16 outputs
    KMShield.digitalWriteKM(led, HIGH);               // Turn led on
    delay(MS_ON);                                     // Wait. LED is on.
    KMShield.digitalWriteKM(led, LOW);                // Now turn it off. 
    delay(MS_OFF);                                    // Wait. It's off.
  } 
}


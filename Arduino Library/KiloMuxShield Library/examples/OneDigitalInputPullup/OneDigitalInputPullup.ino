#include <KiloMux.h>              // Import class declaration
#include <KiloMuxDefs.h>          // Import KiloMux defines

KiloMux KMShield;                 // KiloMux Shield  

unsigned int buttonInput = 9;     // Shield input where we connected a button or a sensor (1-16)
                                  // No need for 10K pull-up resistor in this example

void setup() {
  Serial.begin(115200);           // Initialize serial at 115200bps
}

void loop() {
  int buttonState = 0;                                         // Variable to store digital values
  buttonState = KMShield.digitalReadKM(MUX_B, buttonInput, PULLUP);     // Read digital value from MUX_A and channel 'buttonInput' (1-16), and set internal pullup  
  
  Serial.print("Button state: "); Serial.println(buttonState);          // print value at the serial monitor
}


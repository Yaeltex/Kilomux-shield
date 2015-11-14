#include <KiloMux.h>          // Import class declaration
#include <KiloMuxDefs.h>

KiloMux KMShield;             // KiloMux Shield    

unsigned int potInput = 1;    // Shield input where we connected a potentiometer or a sensor (1-16)

void setup() {
  Serial.begin(115200);       // Initialize serial
}

void loop() {
  int analogData = 0;                                           // Variable to store analog values
  analogData = KMShield.analogReadKM(MUX_A, potInput);          // Read analog value from MUX_A and channel 'potInput' (1-16) 
  
  Serial.print("Pot reading: "); Serial.println(analogData);    // Print value at the serial monitor
}



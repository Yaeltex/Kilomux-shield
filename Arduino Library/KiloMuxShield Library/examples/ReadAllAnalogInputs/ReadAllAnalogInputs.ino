#include <KiloMux.h>          // Import class declaration
#include <KiloMuxDefs.h>      // Import KiloMux defines

KiloMux KMShield;             // KiloMux Shield  

void setup() {
  Serial.begin(115200);       // Initialize serial
}

void loop() {
  int analogData = 0;         // Variable to store analog values
  
  for (int input = MUX_A1_START; input <= MUX_A1_END; input++){      // Sweeps all 8 multiplexer inputs of Mux A1 header
    analogData = KMShield.analogReadKM(MUX_A, input);                // Read analog value from MUX_A and channel 'input'
    
    Serial.print("IN "); Serial.print(input);                        // Print all values in a format like
    Serial.print(": ");                                              //    IN 0: 581  IN 1: 230 ...... IN 8: 500 
    Serial.print(analogData);                                        //    IN 0: 582  IN 1: 230 ...... IN 8: 500
    Serial.print("  ");                                              //    IN 0: 583  IN 1: 230 ...... IN 8: 500
  }                                                                  
  
  Serial.println("");                                                // New Line
}



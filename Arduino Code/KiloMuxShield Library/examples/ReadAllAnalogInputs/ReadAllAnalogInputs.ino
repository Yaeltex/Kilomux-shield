/*
 * Author: Franco Grassano - YAELTEX
 * Date: 18/02/2016
 * ---
 * LICENSE INFO
 * Kilo Mux Shield by Yaeltex is released by
 * Creative Commons Atribución-CompartirIgual 4.0 Internacional - http://creativecommons.org/licenses/by-sa/4.0/
 * ----
 * Description: Reads several analog inputs for any of the shield's ports, and displays them on the serial monitor.
 *              Change MUX_START and MUX_END to read other inputs.
 *              To read MUX_B, change parameter in analogReadKM function.
 *              To read digital sensors, like buttons, instead of analog,change function to digitalReadKM.
 *              This example is for use with the KiloMux Shield.
 * 
 * KiloMux Library is available at https://github.com/Yaeltex/KiloMux-Shield/blob/master/Arduino%20Code/KiloMuxShield%20Library/KiloMuxShield.zip
 */

#include <KiloMux.h>          // Import class declaration
#include <KiloMuxDefs.h>      // Import KiloMux defines

#define MUX_START   1
#define MUX_END     8

KiloMux KMShield;             // KiloMux Shield  

void setup() {
  Serial.begin(115200);       // Initialize serial
}

void loop() {
  int analogData = 0;         // Variable to store analog values
  
  for (int input = MUX_START; input <= MUX_END; input++){      // Sweeps all 8 multiplexer inputs of Mux A1 header
    analogData = KMShield.analogReadKM(MUX_A, input);                // Read analog value from MUX_A and channel 'input'
    
    Serial.print("IN "); Serial.print(input);                        // Print all values in a format like
    Serial.print(": ");                                              //    IN 0: 581  IN 1: 230 ...... IN 8: 500 
    Serial.print(analogData);                                        //    IN 0: 582  IN 1: 230 ...... IN 8: 500
    Serial.print("  ");                                              //    IN 0: 583  IN 1: 230 ...... IN 8: 500
  }                                                                  
  
  Serial.println("");                                                // New Line
}



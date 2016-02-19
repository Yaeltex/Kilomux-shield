/*
 * Author: Franco Grassano - YAELTEX
 * Date: 18/02/2016
 * ---
 * LICENSE INFO
 * Kilomux Shield by Yaeltex is released by
 * Creative Commons Atribución-CompartirIgual 4.0 Internacional - http://creativecommons.org/licenses/by-sa/4.0/
 * ----
 * Description: Analog input reading with threshold filtering to avoid flicker readings.
 *              This example is for use with the KiloMux Shield.
 * 
 * Kilomux Library is available at https://github.com/Yaeltex/Kilomux-Shield/blob/master/Arduino%20Code/KilomuxShield%20Library/KilomuxShield.zip
 */

#include <Kilomux.h>          // Import class declaration
#include <KilomuxDefs.h>      // Import KiloMux defines

Kilomux KmShield;             // Kilomux Shield    

#define ANALOG_INCREASING   0     
#define ANALOG_DECREASING   1

#define NOISE_THRESHOLD     3 

unsigned int potInput = 1;    // Shield input where we connected a potentiometer or a sensor (1-16)

void setup() {
  Serial.begin(115200);       // Initialize serial
}

void loop() {
  int analogData = 0;                                           // Variable to store analog values
  
  analogData = KmShield.analogReadKm(MUX_A, potInput);          // Read analog value from MUX_A and channel 'potInput' (1-16) 
  
  if (IsNoise(analogData)){                                     // If IsNoise() returns 1, then the new analog value is just noise
    // in which case, do nothing
  }
  else{
    Serial.print("Pot reading: "); Serial.println(analogData);  // If IsNoise() returns 0, the value is not noise. Print value to serial monitor  
  }
  
}


// Thanks to Pablo Fullana for the help with this function!
// It's just a threshold filter. If the new value stays within the previous value + - the noise threshold set, then it's considered noise
unsigned int IsNoise(int newValue) {
  static bool analogDirection = 0;            // "static" means the value is stored and can be used next time this function runs
  static unsigned int prevValue = 0;          // Store previous analog value
  
  if (analogDirection == ANALOG_INCREASING){          // CASE 1: If signal is increasing,
    if(newValue > prevValue){                           // and the new value is greater than the previous,
       prevValue = newValue;                            // store new value as previous and return
       return 0;                                        // NOT NOISE!
    }
    else if(newValue < prevValue - NOISE_THRESHOLD){  // If, otherwise, it's lower than the previous value and the noise threshold together,
      analogDirection = ANALOG_DECREASING;              // means it started to decrease,
      prevValue = newValue;                             // so store new value as previous and return    
      return 0;                                         // NOT NOISE!
    }
  }
  if (analogDirection == ANALOG_DECREASING){          // CASE 2: If signal is increasing,                         
    if(newValue < prevValue){                           // and the new value is lower than the previous,
       prevValue = newValue;                            // store new value as previous and return
       return 0;                                        // NOT NOISE!
    }
    else if(newValue > prevValue + NOISE_THRESHOLD){  // If, otherwise, it's greater than the previous value and the noise threshold together,
      analogDirection = ANALOG_INCREASING;              // means it started to increase,
      prevValue = newValue;                             // so store new value as previous and return
      return 0;                                         // NOT NOISE!
    }
  }
  return 1;                                           // If everyting above was not true, then IT'S NOISE! Pot is trying to fool us. But we owned you pot ;)
}

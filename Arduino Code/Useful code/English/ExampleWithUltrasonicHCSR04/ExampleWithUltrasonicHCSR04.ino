/*
 * Author: Franco Grassano - YAELTEX
 * Date: 18/02/2016
 * ---
 * LICENSE INFO
 * Kilomux Shield by Yaeltex is released by
 * Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0) - http://creativecommons.org/licenses/by-sa/4.0/
 * ----
 * Description: Use of an ultrasonic distance sensor HC-SR04. Reads sensor and sends MIDI CC message, whenever it changed.
 *              Uses moving average filter to smooth readings, and prevents reading from going to 0 whenever maximum distance is overreached, 
 *              or object goes out of sensing line.
 *              Uses activation button to turn sensing on or off, and a LED to indicate this.
 *              This example is for use with the Kilomux Shield.
 * 
 * KiloMux Library is available at https://github.com/Yaeltex/Kilomux-Shield/blob/master/Arduino%20Code/KilomuxShield%20Library/KilomuxShield.zip
 */

/*
 * Adding librarys to sketch 
 */
#include <NewPing.h>
#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>
//#include <midi_Message.h>      // Si no compila porque le falta midi_message.h, descomentar esta linea

/*
 * Next is the configuration section.
 * Each #define will vary how the code below works.
 * Each line can be commented out with a double slash at the beggining (//). If a label is commented out, evert line of code between
 *    #ifdef LABEL
 *    ...
 *    ...
 *    ...
 *    #endif
 * will not be present on the final code. The compiler won't use them.
 * 
 * This is a nice thing to do when you don't want code to use memory space in your microcontroller when you don't use 
 * a certain feature.
 */
 
void setup(); // Sometimes this prevents certain pre-processor errors, when using #ifdef
 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Uncomment next line if the compiler doesn't find MIDI.
// MIDI_CREATE_DEFAULT_INSTANCE()
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Leave only one of the following lines uncommented, to define the communication type
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define SERIAL_COMMS      // Debug mode, for use with Serial Monitor
//#define HAIRLESS_MIDI     // Debug mode 2, if you want to send MIDI over Serial, NOT RECOMMENDED
//#define MIDI_USB          // Production mode, to send MIDI via USB
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * In this session, the define labels represent a numeric value
 * The compiler replaces them for their respective values, and they don't use RAM space in your microcontroller, 
 * as variables do.
 * Some of them should not be modified, others you can play with, according to your application
 */

// DEFINES FOR ULTRASONIC SENSOR                                                                                                    
  // MODIFIABLE
  #define CC_ULTRASONIC        100
  #define MAX_DISTANCE         45        // Maximum distance to measure in cms (1 inch = 2.54 cm). HC-SR04 can measure up to 400 cm.
  #define MIN_DISTANCE         2         // Minimum distance to measure in cms (1 inch = 2.54 cm). HC-SR04 can measure from 1 cm.
  #define DELAY_ULTRAS         15        // Delay between sensor pings in milliseconds. Keep over 10 ms
  #define THRES_US_DIFF        80        // 
  #define FILTER_N             3         // Amount of filter values stored. The higher, the better the filtering, and slower response
  #define TRIGGER_PIN          13        // Arduino pin connected to sensor trigger pin
  #define ECHO_PIN             12        // Arduino pin connected to sensor echo pin
  #define PIN_BUTTON_ACT_US    10        // Arduino pin connected to sensor activate/deactivate button
  #define PIN_LED_ACT_US       11        // Arduino pin connected to activation status LED
  // DON'T CHANGE
  #define MAX_US               MAX_DISTANCE*US_ROUNDTRIP_CM   // Maximum time in microseconds that the ultrasonic pulse will take to come back after triggered. 
  #define MIN_US               MIN_DISTANCE*US_ROUNDTRIP_CM   // Minimum time in microseconds that the ultrasonic pulse will take to come back after triggered. 

// MODIFIABLE
#define SERIAL_RATE 115200            // Serial Baud Rate (bits/sec). To use with HAIRLESS MIDI, set 115200. Always remember to set the same speed on the receiving end.

//////////////////////////////////////////////////////////////////////////////////////////
// Variables and class instances /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

// Class instances //////////////////////////////////////////////////////////////////
NewPing sensorUS(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // Instance for NewPing library.

int ccSensorValuePrev[FILTER_N];                      // Filtering array

///////////////////////////////////////////////////////////////////////////////////////////
// Function prototypes ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

// Function for US reading ////////////////////////////////////////////////////////////////
int ReadUltrasonic(void);

///////////////////////////////////////////////////////////////////////////////////////////
/// SETUP /////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  pinMode(PIN_LED_ACT_US, OUTPUT);         // Status LED
  pinMode(PIN_BUTTON_ACT_US,INPUT_PULLUP);  // Activation button

#if defined(MIDI_USB)
  MIDI.begin(MIDI_CHANNEL_OMNI); MIDI.turnThruOff();  // MIDI library instance. MIDI_CHANNEL_OMNI listens to every MIDI channel (1-16).
                                                      // By default, Arduino MIDI Library sets THRU ON. We don't want this, so we turn it OFF.
#elif defined(SERIAL_COMMS)
  Serial.begin(SERIAL_RATE);                          // Serial communication initialization.
#elif defined(HAIRLESS_MIDI)
  MIDI.begin(MIDI_CHANNEL_OMNI); MIDI.turnThruOff();  // MIDI library instance. MIDI_CHANNEL_OMNI listens to every MIDI channel (1-16).
                                                      // By default, Arduino MIDI Library sets THRU ON. We don't want this, so we turn it OFF.
  Serial.begin(SERIAL_RATE);                          // Serial communication initialization.
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
// MAIN LOOP /////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  // Read ultrasonic sensor value, filter and send information over Serial or USB-MIDI.
  int sensorReading = ReadUltrasonic();;
  if (sensorReading > 0){
    #if defined(MIDI_USB)|defined(HAIRLESS_MIDI)
      MIDI.sendControlChange(CC_ULTRASONIC, sensorReading, 1);                         // Send MIDI CC over channel 1
    #elif defined(SERIAL_COMMS)
      Serial.print("Ultrasonic sensor value: "); Serial.println(sensorReading);        // Send value over SERIAL
    #endif
  }  
}

int ReadUltrasonic(void){
  // Static variables keep their values next time this function is called.
  static unsigned int prevMillisUS = 0;         // Previous millis since last ping.
  static bool prevUSbuttonState = 1;            // Previous activation button state (Active low).
  static bool currUSbuttonState = 1;            // Current activation button state (Active low).
  static bool statusLED = 0;                    // Status LED state.
  static bool sensorOn = 0;                     // Internal variable to set sensor ON or OFF.
  static unsigned int filterIndex = 0;          // Smoothing filter index.
    
  int microSec = 0;                             // Time in microseconds the ultrasonic pulse takes to come back after triggered.
  
  // This code checks if the button is pressed, activates or deactivates the sensor reading.
  currUSbuttonState = digitalRead(PIN_BUTTON_ACT_US);
  if(currUSbuttonState == LOW && prevUSbuttonState == HIGH){  // If button was previously HIGH and now is LOW, it means it is now pressed. Each time it is pressed, will switch the sensor ON or OFF.
    prevUSbuttonState = LOW;                                   // Update previous state.
    sensorOn = !sensorOn;                                     // Activate or deactivate sensor.
    statusLED = !statusLED;                                   // Change status LED state.
    digitalWrite(PIN_LED_ACT_US, statusLED);                  // Turn LED ON or OFF.
  }
  else if(currUSbuttonState == HIGH && prevUSbuttonState == LOW){   // If button was previously LOW and now is HIGH, it means it is now released. 
    prevUSbuttonState = HIGH;                                       // Update previous state.
  } 
  
  if(sensorOn){                                                                           // If sensor is ON
    if (millis()-prevMillisUS > DELAY_ULTRAS){                                              // and the desired delay between pings is transpired
      prevMillisUS = millis();                                                                // Update current milliseconds
      microSec = sensorUS.ping();                                                             // ping() method triggers an ultrasonic pulse and waits for it to return. This can take up to 10 ms. Be careful if you are time constraint.
      int ccSensorValue = map(constrain(microSec, MIN_US, MAX_US), MIN_US, MAX_US, 0, 130);   // This line converts the value given by the ping() method in milliseconds to a number between 0 a 127.

      // Smoothing Filter
      if(ccSensorValue != ccSensorValuePrev[filterIndex]){             // If current value is different to previous value, filter
        if(!ccSensorValue & ccSensorValuePrev[filterIndex] > 10){        // This "if" prevents value from going to 0 when the object being measured is removed or exceeds maximum distance
          ccSensorValue =  ccSensorValuePrev[filterIndex];               // 
        }
        // MOVING AVERAGE FILTER
        for(int i = 0; i < FILTER_N; i++){                      
          ccSensorValue += ccSensorValuePrev[i];           // Adds to current value, FILTER_N previous values
        }
        
        ccSensorValue /= FILTER_N+1;                      // Divide by N+1
        ccSensorValuePrev[filterIndex++] = ccSensorValue; // And update circular buffer index
        filterIndex %= FILTER_N+1;
        // FIN FILTRADO ////////////////////////////////
        
        // If there were any changes in the filtered value, return value. This "if" prevents several same-value messages.
        if(ccSensorValue != ccSensorValuePrev[filterIndex])      
          return ccSensorValue;
      }
      else return -1;                      // If sensor is OFF, return -1
    }
  }
}

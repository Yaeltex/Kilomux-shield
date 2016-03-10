/*
 * Author: Franco Grassano - YAELTEX
 * Date: 18/02/2016
 * ---
 * LICENSE INFO
 * Kilomux Shield by Yaeltex is released by
 * Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0) - http://creativecommons.org/licenses/by-sa/4.0/
 * ----
 * Description: This code drives an LED matrix with the outputs of the Kilomux shield.
 *              The example can be used with up to 8 rows and 8 columns, setting the ROWS and COLS labels accordingly.
 *              It's important to drive the columns and the rows with different ports (OUT-1 and OUT-2), and to specify 
 *              the port used for the columns in the label COL_PORT.
 * 
 * KiloMux Library is available at https://github.com/Yaeltex/Kilomux-Shield/blob/master/Arduino%20Code/KilomuxShield%20Library/KilomuxShield.zip
 */
 
#include <Kilomux.h>
#include <KilomuxDefs.h>

#define ROW_INTERVAL 2500                 // Row multiplexing interval in microseconds. With about 2.5 ms, no human can percept the multiplexing. You can't. I can't. No one can.
#define ROWS      4                       // Number of matrix rows. Change for your setup.
#define COLS      4                       // Number of matrix columns. Change for your setup.

#define COL_PORT  2                       // Output port where the columns are connected

#define N_LEDS    ROWS*COLS               // Total number of LEDs

#define LED_OFF   0
#define LED_ON    1

#define BLINK_INTERVAL  300                // Blinking interval in milliseconds

Kilomux KmShield;                          // Kilomux instance

unsigned int ledRows[] = {1, 2, 3, 4};     // KiloMux's outputs where we connected the rows. Add or take if you have a different setup.
unsigned int ledCols[] = {9, 10, 11, 12};  // KiloMux's outputs where we connected the columns. Add or take if you have a different setup.

uint8_t ledMatrixState[ROWS][COLS];        // This matrix will match always the LED matrix state.
uint8_t led = 0;                           // Counter
unsigned long lastMillis = 0;              // Variable to store time. Always type long, since it will have large values.

void setup() {
  // Hardware initialization
  for (int i=0; i<ROWS; i++){
    KmShield.digitalWriteKm(ledRows[i], LOW);       // Rows to LOW, means row off.
  }
  KmShield.digitalWritePortKm(B11111111, COL_PORT); // Columns to HIGH, means columns off.

  // Software initialization
  for(int i = 0; i<N_LEDS; i++){
    setMatrixLed(i,LED_OFF);            // Set all LEDs off
  }

  setMatrixLed(0, LED_ON);            // Set LED in row 0, column 0 on
  // Get current time
  lastMillis = millis();      // Get current time
}

void loop() {
  if(millis() - lastMillis > BLINK_INTERVAL){     // If blinking interval is passed,
    lastMillis = millis();                          // Get current time
    setMatrixLed(led, LOW);                         // Set previous led off
    led++;                                          // Advance counter
    led %= N_LEDS;                                  // Circular counter update
    setMatrixLed(led, HIGH);                        // Set new led on
  }
  
  // OUTPUT UPDATE //////////////////////////////////////////////
  updateLedMatrix();
}

void setMatrixLed(uint8_t nLed, uint8_t state){   
  ledMatrixState[nLed/COLS][nLed%COLS] = state;     // With this line i transform a number into a coordinate (row, column). 
  return;                                           // For example, number 5, with 4 rows and 4 columns -> row = 5/4 = 1 and col = 5%4 = 1 -> so led 5 will be at row 1, column 1, which is correct.
}                                                   //              number 13, with 2 rows and 8 columns -> row = 13/8 = 1 and col = 13%8 = 5 -> so led 13 will be at row 1, column 5, which is correct.

/*
 * This function performs row multiplexing. That means updating one row at a time, but very fast, so humans will not percieve it.
 * Instead, we will see every led continuously on or off.
 * 
 */
void updateLedMatrix(void){
  static unsigned long prevMicros = 0;                // Time stamp for row update
  static unsigned int row;                            // Current row
  byte colState = 0;                                  // Byte to update every column at once

  unsigned long currentMicros = micros();             // Get current time
  if(currentMicros-prevMicros > ROW_INTERVAL){        // If it is time to move to the next row
    prevMicros = currentMicros;                         // Save current time
    KmShield.digitalWriteKm(ledRows[row++], LOW);       // Set previous row off and advance row counter
    row %= ROWS;                                        // If last row is reached, update circular counter
    colState &= 0x00;                                   // Set byte to B00000000
    for(int col = 0; col<COLS; col++){                  // For every column
      int thisLed = ledMatrixState[row][col];             // Get led state
      colState |= thisLed<<(7-col);                       // This line will "or" a 1 in the right column. 
    }                                                     // For example, if column 5 (starting from 0) needs to be set on, this will set colState to B00000100 (1<<7-5)
    KmShield.digitalWritePortKm(~colState, COL_PORT); // Columns need to be set to LOW in order for the LEDs to light up, so if the column 5 is the only one to be turned on, the port will be set to B11111011
    KmShield.digitalWriteKm(ledRows[row], HIGH);      // Turn current row on, lighting up the LEDs
  }  
  return;
}

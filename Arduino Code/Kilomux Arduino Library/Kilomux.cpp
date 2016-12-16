/*
 * Author: Franco Grassano - YAELTEX
 * Date: 18/02/2016
 * Buenos Aires, Argentina - 2015
 * ---
 * LICENSE INFO
 * Kilomux Shield library for Arduino, by Yaeltex, is released by
 * Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0) - http://creativecommons.org/licenses/by-sa/4.0/
 * ----
 *
 * Definition of methods of the Kilomux class
 *
 */

#include "Arduino.h"
#include "Kilomux.h"

/*
  Method:         Kilomux
  Description:    Class constructor. Sets everything for a Kilomux Shield to work properly.
  Parameters:     void
  Returns:        void
*/
Kilomux::Kilomux(){
    
}

/*
  Method:         initialize
  Description:    Set everything up (pins, ADC speed, initialize variables and prepare shift registers)
  Parameters:     void
  Returns:        void
*/
void Kilomux::init(void){
	// Set output pins for shift register
    pinMode(LatchPin, OUTPUT);
    pinMode(DataPin, OUTPUT);
    pinMode(ClockPin, OUTPUT);

    // Set output pins for multiplexers
    pinMode(_S0, OUTPUT);
    pinMode(_S1, OUTPUT);
    pinMode(_S2, OUTPUT);
    pinMode(_S3, OUTPUT);
											  
    // Set every reading to 0
    for (int mux = 0; mux < NUM_MUX; mux++) {
      for (int chan = 0; chan < NUM_MUX_CHANNELS; chan++) {
        muxReadings[mux][chan] = 0;
        muxPrevReadings[mux][chan] = 0;
      }
    }
    for (int output = 0; output < NUM_OUTPUTS; output++) {
       outputState[output] = 0;
    }
    
	// This method sets the ADC prescaler, to change the sample rate
    setADCprescaler(PS_16);               // PS_16 (1MHz or 50000 samples/s)
                                          // PS_32 (500KHz or 31250 samples/s)
                                          // PS_64 (250KHz or 16666 samples/s)
                                          // PS_128 (125KHz or 8620 samples/s)

	
    clearRegisters595();                  // Set all outputs to LOW
    writeRegisters595();                  // Update outputs
}
/*
  Method:         digitalWriteKm
  Description:    Change the state of a shift register output to HIGH or LOW
  Parameters:
                  output - Number of the output affected. Must be within 1 and 16
                  state  - New state of the output. Must be HIGH or LOW (1 or 0)
  Returns:        void
*/
void Kilomux::digitalWriteKm(int output, int state){
  if (output >= 0 && output <= 15){
    outputState[OutputMapping[output]] = state;
    writeRegisters595();
  }
  else return;    // Return not setting any output

}

/*
  Method:         digitalWritePortKm
  Description:    Set the state of an entire output port (8 pins of the 10-pin outpute headers on the shield)
  Parameters:
                  portState - 	Byte containing the state of the port to be set. MSB is output 1 and LSB is output 8 of the port
								portState -> B 1-0-0-1-0-1-1-1
								outputs   ->   1-2-3-4-5-6-7-8
                  port  - 	  	Number of output port to set (1 or 2)
  Returns:        void
*/
void Kilomux::digitalWritePortKm(byte portState, int port){
  if (port == 1 || port == 2){
	  for(int output = 0;  output < 8; output++){
		outputState[OutputMapping[output + (port-1)*8]] = portState&(1<<(7-output));
	  }
	  writeRegisters595();
	}
  return;

}

/*
  Method:         digitalWritePortKm
  Description:    Set the state of an entire output port (8 pins of the 10-pin outpute headers on the shield)
  Parameters:
                  portState - 	Byte containing the state of the port to be set. MSB is output 1 and LSB is output 8 of the port
								portState -> B 1-0-0-1-0-1-1-1
								outputs   ->   1-2-3-4-5-6-7-8
                  port  - 	  	Number of output port to set (1 or 2)
  Returns:        void
*/
void Kilomux::digitalWritePortsKm(byte portState1, byte portState2){
  for(int output = 0;  output < 8; output++){
	outputState[OutputMapping[output]] = portState1&(1<<(7-output));
  }
  for(int output = 8;  output < 16; output++){
	outputState[OutputMapping[output]] = portState2&(1<<(15-output));
  }
  writeRegisters595();
  return;
}

/*
  Method:         digitalReadKm
  Description:    Read the state of a multiplexer channel as a digital input.
  Parameters:
                  mux    - Identifier of the multiplexer desired. Must be MUX_A or MUX_B (0 or 1)
                  chan   - Number of the channel desired. Must be within 1 and 16
  Returns:        int    - 0: input is LOW
                           1: input is HIGH
                          -1: mux or chan is not valid
*/
int Kilomux::digitalReadKm(int mux, int chan){
    int digitalState;
    if (chan >= 0 && chan <= 15){
      if (mux == MUX_A)
        chan = MuxAMapping[chan];
      else if (mux == MUX_B)
        chan = MuxBMapping[chan];
      else return -1;   // Return ERROR
    }
    else return -1;     // Return ERROR

	PORTD &= 0xC3;
	PORTD |= chan<<2;

    switch (mux) {
        case MUX_A:
            digitalState = digitalRead(InMuxA);
            break;
        case MUX_B:
            digitalState = digitalRead(InMuxB);
            break;

        default:
            break;
    }
    return digitalState;
}

/*
  Method:         digitalReadKm
  Description:    Read the state of a multiplexer channel as a digital input, setting the analog input of the Arduino with a pullup resistor.
  Parameters:
                  mux    - Identifier for the desired multiplexer. Must be MUX_A or MUX_B (0 or 1)
                  chan   - Number for the desired channel. Must be within 1 and 16
  Returns:        int    - 0: input is LOW
                           1: input is HIGH
                          -1: mux or chan is not valid
*/
int Kilomux::digitalReadKm(int mux, int chan, int pullup){
    int digitalState;
    if (chan >= 0 && chan <= 15){
      if (mux == MUX_A)
        chan = MuxAMapping[chan];
      else if (mux == MUX_B)
        chan = MuxBMapping[chan];
      else return -1;   // Return ERROR
    }
    else return -1;     // Return ERROR

	PORTD &= 0xC3;
	PORTD |= chan<<2;
	
    switch (mux) {
        case MUX_A:
            pinMode(MUX_A_PIN, INPUT);                // These two lines set the analog input pullup resistor
            digitalWrite(MUX_A_PIN, HIGH);            
            digitalState = digitalRead(InMuxA);		  // Read mux pin
            break;
        case MUX_B:
            pinMode(MUX_B_PIN, INPUT);                // These two lines set the analog input pullup resistor
            digitalWrite(MUX_B_PIN, HIGH);            
            digitalState = digitalRead(InMuxB);		  // Read mux pin
            break;

        default:
            break;
    }
    return digitalState;
}


/*
  Method:         analogReadKm
  Description:    Read the analog value of a multiplexer channel as an analog input.
  Parameters:
                  mux    - Identifier of the multiplexer desired. Must be MUX_A or MUX_B (0 or 1)
                  chan   - Number of the channel desired. Must be within 1 and 16
  Returns:        int    - 0..1023: analog value read
                          -1: mux or chan is not valid
*/
int Kilomux::analogReadKm(int mux, int chan){
    static unsigned int analogData;
    if (chan >= 0 && chan <= 15){     // Re-map hardware channels to have them read in the header order
      if (mux == MUX_A)
        chan = MuxAMapping[chan];
      else if (mux == MUX_B)
        chan = MuxBMapping[chan];
      else return -1;     // Return ERROR
    }
    else return -1;       // Return ERROR

	PORTD &= 0xC3;
	PORTD |= chan<<2;

    switch (mux) {
        case MUX_A:
            analogData = analogRead(InMuxA);
            analogData = analogRead(InMuxA);   // Second reading to prevent analog crosstalk
            break;
        case MUX_B:
            analogData = analogRead(InMuxB);
            analogData = analogRead(InMuxB);   // Second reading to prevent analog crosstalk
            break;
        default:
            break;
    }

    return analogData;
}

/*
  Method:         clearRegisters595
  Description:    Set every output of the shift register to LOW
  Parameters:     void
  Returns:        void
*/
void Kilomux::clearRegisters595(void) {
  for (int i = NUM_OUTPUTS - 1; i >=  0; i--) {
    outputState[i] = LOW;
  }
}

/*
  Method:         writeRegisters595
  Description:    Update the shift registers outputs, sending serial data to them
  Parameters:     void
  Returns:        void
*/
void Kilomux::writeRegisters595() {
  digitalWrite(LatchPin, LOW);                  // Latch line goes LOW to inform the IC that data will start to flow into it.

  for (int i = NUM_OUTPUTS - 1; i >=  0; i--) {    // Cycle through all outputs
    digitalWrite(ClockPin, LOW);                  	// "Manual" clock signal goes LOW

    int val = outputState[i];                       // Each output state is recovered

    digitalWrite(DataPin, val);                   	// And written to the data signal
    digitalWrite(ClockPin, HIGH);                 	// "Manual" clock signal goes HIGH
  }
  digitalWrite(LatchPin, HIGH);                 // Latch line goes HIGH to inform the IC that data is over.
}

/*
  Method:         setADCprescaler
  Description:    Set the sample rate of the ADC. This allows for faster and more readings every second.
                  Atmel recommends keepíng the frequency of operation of the ADC between 50 KHz and 200 KHz,
                  warning that resolution may be compromised if the frequency is higher.
  Parameters:
                  divider   - Desired prescaler value. Can be PS_16 (1MHz or 50000 samples/s)
                                                              PS_32 (500KHz or 31250 samples/s)
                                                              PS_64 (250KHz or 16666 samples/s)
                                                              PS_128 (125KHz or 8620 samples/s)
  Returns:        void
*/
void Kilomux::setADCprescaler(byte divider){
  ADCSRA &= ~PS_128;    // Set register to 0
  ADCSRA |= divider;    // Set the desired prescaler value
}

/*
  Method:         isNoise
  Description:    This method allows to determine if the analog value read is a true reading or just noise.
                  In order to do this, the previous read value must be provided, and the direction in which the value is moving.
                  If the value jumps between the previous value, inside a window of prevValue +- NOISE_THR, then the new reading is considered noise.
                  If the analog voltage is increasing, then the new value needs to be greater than the previous, in order to be considered true.
                  If the analog voltage is decreasing, then the new value needs to be lower than the previous, in order to be considered true.
  Parameters:
                  newValue   - Present reading of the analog value
                  prevValue  - Previous reading of the analog value
                  direction  - Direction of change of the analog value. Must be ANALOG_UP or ANALOG_DOWN
  Returns:        unsigned int - 0: New value is a true reading, not noise.
                                 1: New value was considered to be noise.
*/
unsigned int Kilomux::isNoise(unsigned int newValue, unsigned int prevValue, bool direction) {
  if (direction == ANALOG_UP){                    // If the signal is increasing
    if(newValue > prevValue){                     // and the new value is larger than the previous
       return 0;                                  // it's not noise.
    }
    else if(direction < prevValue - NOISE_THR){   // If the signal is increasing, and the new value is lower than the previous, minus the threshold
      return 0;                                   // it's not noise.
    }
  }
  if (direction == ANALOG_DOWN){                  // If the signal is decreasing
    if(newValue < prevValue){                     // and the new value is lower than the previous
       return 0;                                  // it's not noise.
    }
    else if(newValue > prevValue + NOISE_THR){    // If the signal is decreasing, and the new value is larger than the previous, plus the threshold
      return 0;                                   // it's not noise.
    }
  }
  return 1;         // If arrived here, the new reading isn't noise.
}

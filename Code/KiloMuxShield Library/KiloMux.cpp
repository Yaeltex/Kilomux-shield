#include "Arduino.h"
#include "KiloMux.h"

/*
  Method:         KiloMux
  Description:    Class constructor. Sets everything for a KiloMux Shield to work properly.
  Parameters:     void
  Returns:        void
*/
KiloMux::KiloMux(){
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
    for (int output = 0; output < NUM_SR_OUTPUTS; output++) {
       outputState[output] = 0;
    }
    // This method sets the ADC prescaler, to change the sample rate
    setADCprescaler(PS_16);               // PS_16 (1MHz o 50000 muestras/s)
                                          // PS_32 (500KHz o 31250 muestras/s)
                                          // PS_64 (250KHz o 16666 muestras/s)
                                          // PS_128 (125KHz o 8620 muestras/s)

    clearRegisters595();                  // Set all outputs to LOW
    writeRegisters595();                  // Update outputs
}

/*
  Method:         digitalWriteKM
  Description:    Change the state of a shift register output to HIGH or LOW
  Parameters:
                  output - Number of the output affected. Must be within 1 and 16
                  state  - New state of the output. Must be HIGH or LOW (1 or 0)
  Returns:        void
*/
void KiloMux::digitalWriteKM(int output, int state){
  if (output >= 1 && output <= 16){
    outputState[OutputMapping[output-1]] = state;
    writeRegisters595();
  }
  else return;    // Return not setting any output

}

/*
  Method:         digitalReadKM
  Description:    Read the state of a multiplexer channel as a digital input.
  Parameters:
                  mux    - Identifier of the multiplexer desired. Must be MUX_A or MUX_B (0 or 1)
                  chan   - Number of the channel desired. Must be within 1 and 16
  Returns:        int    - 0: input is LOW
                           1: input is HIGH
                          -1: mux or chan is not valid
*/
int KiloMux::digitalReadKM(int mux, int chan){
    int digitalState;
    if (chan >= 1 && chan <= 16){
      if (mux == MUX_A)
        chan = MuxAMapping[chan-1];
      else if (mux == MUX_B)
        chan = MuxBMapping[chan-1];
      else return -1;   // Return ERROR
    }
    else return -1;     // Return ERROR

    digitalWrite(_S0, (chan&1));
    digitalWrite(_S1, (chan&3)>>1);
    digitalWrite(_S2, (chan&7)>>2);
    digitalWrite(_S3, (chan&15)>>3);

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
  Method:         digitalReadKM
  Description:    Read the state of a multiplexer channel as a digital input.
  Parameters:
                  mux    - Identifier of the multiplexer desired. Must be MUX_A or MUX_B (0 or 1)
                  chan   - Number of the channel desired. Must be within 1 and 16
  Returns:        int    - 0: input is LOW
                           1: input is HIGH
                          -1: mux or chan is not valid
*/
int KiloMux::digitalReadKM(int mux, int chan, int pullup){
    int digitalState;
    if (chan >= 1 && chan <= 16){
      if (mux == MUX_A)
        chan = MuxAMapping[chan-1];
      else if (mux == MUX_B)
        chan = MuxBMapping[chan-1];
      else return -1;   // Return ERROR
    }
    else return -1;     // Return ERROR

    digitalWrite(_S0, (chan&1));
    digitalWrite(_S1, (chan&3)>>1);
    digitalWrite(_S2, (chan&7)>>2);
    digitalWrite(_S3, (chan&15)>>3);

    switch (mux) {
        case MUX_A:
            pinMode(MUX_A_PIN, INPUT);                // Estas dos líneas setean el pin analógico que recibe las entradas digitales como pin digital y
            digitalWrite(MUX_A_PIN, HIGH);            // setea el resistor de Pull-Up en el mismo
            digitalState = digitalRead(InMuxA);
            break;
        case MUX_B:
            pinMode(MUX_B_PIN, INPUT);                // Estas dos líneas setean el pin analógico que recibe las entradas digitales como pin digital y
            digitalWrite(MUX_B_PIN, HIGH);            // setea el resistor de Pull-Up en el mismo
            digitalState = digitalRead(InMuxB);
            break;

        default:
            break;
    }
    return digitalState;
}


/*
  Method:         analogReadKM
  Description:    Read the analog value of a multiplexer channel as an analog input.
  Parameters:
                  mux    - Identifier of the multiplexer desired. Must be MUX_A or MUX_B (0 or 1)
                  chan   - Number of the channel desired. Must be within 1 and 16
  Returns:        int    - 0..1023: analog value read
                          -1: mux or chan is not valid
*/
int KiloMux::analogReadKM(int mux, int chan){
    static unsigned int analogData;
    if (chan >= 1 && chan <= 16){     // Re-map hardware channels to have them read in the header order
      if (mux == MUX_A)
        chan = MuxAMapping[chan-1];
      else if (mux == MUX_B)
        chan = MuxBMapping[chan-1];
      else return -1;     // Return ERROR
    }
    else return -1;       // Return ERROR

    digitalWrite(_S0, (chan&1));
    digitalWrite(_S1, (chan&3)>>1);
    digitalWrite(_S2, (chan&7)>>2);
    digitalWrite(_S3, (chan&15)>>3);

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
  Description:    Set every output to LOW
  Parameters:     void
  Returns:        void
*/
void KiloMux::clearRegisters595(void) {
  for (int i = NUM_SR_OUTPUTS - 1; i >=  0; i--) {
    outputState[i] = LOW;
  }
}

/*
  Method:         writeRegisters595
  Description:    Update the shift registers outputs, sending serial data to them
  Parameters:     void
  Returns:        void
*/
void KiloMux::writeRegisters595() {
  digitalWrite(LatchPin, LOW);                  // Se baja la línea de latch para avisar al 595 que se van a enviar datos

  for (int i = NUM_SR_OUTPUTS - 1; i >=  0; i--) {    // Se recorren todos los LEDs,
    digitalWrite(ClockPin, LOW);                  // Se baja "manualmente" la línea de clock

    int val = outputState[i];                       // Se recupera el estado del LED

    digitalWrite(DataPin, val);                   // Se escribe el estado del LED en la línea de datos
    digitalWrite(ClockPin, HIGH);                 // Se levanta la línea de clock para el próximo bit
  }
  digitalWrite(LatchPin, HIGH);                 // Se vuelve a poner en HIGH la línea de latch para avisar que no se enviarán mas datos
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
void KiloMux::setADCprescaler(byte divider){
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
unsigned int KiloMux::isNoise(unsigned int newValue, unsigned int prevValue, bool direction) {
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
  return 1;         // Si todo lo anterior no se cumple, es ruido.
}

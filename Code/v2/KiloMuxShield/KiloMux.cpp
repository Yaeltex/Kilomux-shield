#include "Arduino.h"
#include "KiloMux.h"


KiloMux::KiloMux(bool ultrasonicUsed)
{
    _S0 = 2;
    _S1 = 3;
    _S2 = 4;
    _S3 = 5;

    DataPin = 6;
    LatchPin = 7;
    ClockPin = 8;

    ActivateButtonPin = 10;
    ActivatedLedPin = 11;
    EchoPin = 12;
    TriggerPin = 13;

    // Set output pins for shift register
    pinMode(LatchPin, OUTPUT);
    pinMode(DataPin, OUTPUT);
    pinMode(ClockPin, OUTPUT);

    if(ultrasonicUsed){
      pinMode(PIN_LED_ACT_US, OUTPUT);         // LED activation indicator for ultrasonic sensor
      pinMode(PIN_BOTON_ACT_US,INPUT_PULLUP);  // Activation button for ultrasonic sensor
    }

    // Setear todos las lecturas a 0
    for (int mux = 0; mux < NUM_MUX; mux++) {
      for (int chan = 0; chan < NUM_MUX_CHANNELS; chan++) {
        muxReadings[mux][canal] = 0;
        muxPrevReadings[mux][canal] = 0;
      }
    }
    for (int outputs = 0; outputs < NUM_OUTPUTS; j++) {
       registers[led] = 0;
    }
    // Ésta función setea la velocidad de muestreo del conversor analógico digital.
    SetADCprescaler(PS_16);

    clearRegisters595
}


void KiloMux::digitalWriteKM(int SROutput, int state){
  setLed595(SROutput,state);
  writeRegisters595();
}


int KiloMux::digitalReadKM(int mux, int chan)
{
    int val;
    switch (mux) {
        case 1:
            digitalWrite(_S0, (chan&1));
            digitalWrite(_S1, (chan&3)>>1);
            digitalWrite(_S2, (chan&7)>>2);
            digitalWrite(_S3, (chan&15)>>3);

            val = digitalRead(InMuxA);
            break;
        case 2:
            digitalWrite(_S0, (chan&1));
            digitalWrite(_S1, (chan&3)>>1);
            digitalWrite(_S2, (chan&7)>>2);
            digitalWrite(_S3, (chan&15)>>3);

            val = digitalRead(InMuxB);
            break;

        default:
            break;
    }
    return val;
}

int KiloMux::analogReadKM(int mux, int chan)
{
   // digitalWrite(_OUTMD,LOW);
    int val;

    digitalWrite(_S0, (chan&1));
    digitalWrite(_S1, (chan&3)>>1);
    digitalWrite(_S2, (chan&7)>>2);
    digitalWrite(_S3, (chan&15)>>3);

    switch (mux) {
        case 1:
            val = analogRead(InMuxA);
            val = analogRead(InMuxA);   // Second reading to prevent analog crosstalk
            break;
        case 2:
            val = analogRead(InMuxB);
            val = analogRead(InMuxB);   // Second reading to prevent analog crosstalk
            break;
        default:
            break;
    }
    return val;
}

// Prender o apagar un solo led mediante el
void KiloMux::setOutput595(int num_out, int state) {
  registros[num_out] = state;
  writeRegisters595();
}

// Apagar todos los LEDs
void KiloMux::clearRegisters595() {
  for (int i = NUM_LEDS_X_BANCO - 1; i >=  0; i--) {
    registers[i] = LOW;
  }
}

// Actualizar el registro de desplazamiento. Actualiza el estado de los LEDs
// Antes de llamar a ésta función, el array 'registros' debería contener el estado requerido para TODOS los LEDs
void KiloMux::writeRegisters595() {
  digitalWrite(latchPin, LOW);                  // Se baja la línea de latch para avisar al 595 que se van a enviar datos

  for (int i = NUM_LEDS_X_BANCO - 1; i >=  0; i--) {    // Se recorren todos los LEDs,
    digitalWrite(clockPin, LOW);                  // Se baja "manualmente" la línea de clock

    int val = registers[i];                       // Se recupera el estado del LED

    digitalWrite(DataPin, val);                   // Se escribe el estado del LED en la línea de datos
    digitalWrite(ClockPin, HIGH);                 // Se levanta la línea de clock para el próximo bit
  }
  digitalWrite(LatchPin, HIGH);                 // Se vuelve a poner en HIGH la línea de latch para avisar que no se enviarán mas datos
}

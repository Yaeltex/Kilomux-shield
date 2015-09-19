/*
KiloMux Library
 */
#ifndef KiloMux_h
#define KiloMux_h

#include "KiloMuxDefs.h"

class KiloMux
{
public:
    KiloMux();
    KiloMux(bool ultrasonicUsed);
    void digitalWriteKM(int led, int state);                       // Write digital data from multiplexer
    int digitalReadKM(int mux, int chan);               // Read digital data from multiplexer
    int analogReadKM(int mux, int chan);                // Read analog data from multiplexer
    int US_SensorRead();                                // Read ultrasonic sensor data
    void SetADCprescaler(byte setting);                 // Set Analog to Digital Converter Prescaler to make analog readings faster
private:
    // Functions
    void setOutput595(int num_out, int state);
    void clearRegisters595();
    void writeRegisters595();

    // Address lines for multiplexer
  	const int _S0; _S1; _S2; _S3;
  	// Input line for multiplexer
    const int InMuxA = A0; InMuxB = A1;

  	// Shift register lines
  	const int DataPin;                             // Data Pin (DS) connected to pin 14 of 74HC595
  	const int LatchPin;                            // Latch Pin (ST_CP) connected to pin 12 of 74HC595
  	const int ClockPin;                            // Clock Pin (SH_CP) connected to pin 11 of 74HC595

    // Ultrasonic Sensor lines
    const int ActivateButtonPin;
    const int ActivatedLedPin;
    const int EchoPin;
    const int TriggerPin;

    // Mux readings
    unsigned int muxReadings[NUM_MUX][NUM_MUX_CHANNELS];
    unsigned int muxPrevReadings[NUM_MUX][NUM_MUX_CHANNELS];

    // Shift register output variables
    bool registers[8];       // For use with the
    uint8_t estadoLeds[BANCOS][NUM_LEDS_X_BANCO]; // Estado de todos los bancos de LEDs

};

#endif

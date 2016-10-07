/*
 * Autor: Franco Grassano - YAELTEX
 * ---
 * INFORMACIÓN DE LICENCIA
 * Kilo Mux Shield por Yaeltex se distribuye bajo una licencia
 * Creative Commons Atribución-CompartirIgual 4.0 Internacional - http://creativecommons.org/licenses/by-sa/4.0/
 * ----
 * Código para el manejo de los integrados 74HC595 tomado de http://bildr.org/2011/02/74HC595/
 * Librería de multiplexado (modificada) tomada de http://mayhewlabs.com/products/mux-shield-2
 * Librería para el manejo del sensor de ultrasonido tomada de http://playground.arduino.cc/Code/NewPing
 * 
 * Este código fue desarrollado para el KILO MUX SHIELD desarrolado en conjunto por Yaeltex y el Laboratorio del Juguete, en Buenos Aires, Argentina, 
 * apuntando al desarrollo de controladores MIDI con arduino.
 * Está preparado para manejar 2 (expandible) registros de desplazamiento 74HC595 conectados en cadena (16 salidas digitales en total),
 * y 2 multiplexores CD4067 de 16 canales cada uno (16 entradas analógicas, y 16 entradas digitales), pero es expandible en el
 * caso de utilizar hardware diferente. Para ello se modifican los "define" NUM_MUX, NUM_CANALES_MUX y NUM_595s. 
 * NOTA: Se modificó la librería MuxShield, para trabajar sólo con 1 o 2 multiplexores. Si se necesita usar más multiplexores, descargar la librería original.
 * 
 * Para las entradas analógicas, por cuestiones de ruido se recomienda usar potenciómetros o sensores con buena estabilidad, y con preferencia con valores
 * cercanos o menores a 10 Kohm.
 * Agradecimientos:
 * - Jorge Crowe
 * - Lucas Leal
 * - Dimitri Diakopoulos
 */

/*
 * Inclusión de librerías. 
 */

#include <KM_Data.h>
#include <Kilomux.h>
#include "Comms.h"
#include "Inputs.h"


void setup(); // Esto es para solucionar el bug que tiene Arduino al usar los #ifdef del preprocesador

#define BLINK_INTERVAL 50

KMS::GlobalData gD = KMS::globalData();

Kilomux KMShield;                                       // Objeto de la clase Kilomux
   
// Contadores, flags //////////////////////////////////////////////////////////////////////
byte i, j, mux, canal;                       // Contadores para recorrer los multiplexores
byte numBanks, numInputs, numOutputs;
bool ledMode;
bool flagBlink = 0, configMode = 0, receivingSysEx = 0;                         
unsigned long millisPrevLED = 0;               // Variable para guardar ms
static byte blinkCount = 0;
unsigned int packetSize;
///////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  #if defined(MIDI_COMMS)
  MIDI.begin(MIDI_CHANNEL_OMNI); // Se inicializa la comunicación MIDI.
  MIDI.turnThruOff();            // Por default, la librería de Arduino MIDI tiene el THRU en ON, y NO QUEREMOS ESO!
  #else
  Serial.begin(115200);                  // Se inicializa la comunicación serial.
  #endif
  
  KMS::initialize();

  pinMode(13, OUTPUT);

  if(gD.protocolVersion() == 1) 
    packetSize = 200;
    
  ResetConfig();
}

void loop(){
  
  #if defined(MIDI_COMMS)
  ReadMidi();
  #endif

  blinkLED();
  
  if(!receivingSysEx) ReadInputs();
}

void ResetConfig(void){
  if(gD.isValid()){
    ledMode = gD.hasOutputMatrix();
    numBanks = gD.numBanks();
    numInputs = gD.numInputsNorm();
    numOutputs = gD.numOutputs();
    #if !defined(MIDI_COMMS)
    Serial.print("Numero de bancos: "); Serial.println(numBanks);
    Serial.print("Numero de entradas: "); Serial.println(numInputs);
    Serial.print("Numero de salidas: "); Serial.println(numOutputs);
    #endif
  }

  KMS::setBank(0); 
  
  // Setear todos las lecturas a 0
  for (int mux = 0; mux < NUM_MUX; mux++) {                
    for (canal = 0; canal < NUM_MUX_CHANNELS; canal++) {
      KMShield.muxReadings[mux][canal] = KMShield.analogReadKm(mux, canal);
      KMShield.muxPrevReadings[mux][canal] = KMShield.muxReadings[mux][canal];
      digitalInputState[canal+mux*NUM_MUX_CHANNELS] = 0;
    }
  }
}

void blinkLED(){
  static bool lastLEDState = LOW;
  static unsigned long millisPrev = 0;
  static bool firstTime = true;
  
  if(flagBlink && blinkCount){
    if (firstTime){ firstTime = false; millisPrev = millis(); }
    if(millis()-millisPrev > BLINK_INTERVAL){
      millisPrev = millis();
      digitalWrite(13, !lastLEDState);
      lastLEDState = !lastLEDState;
      if(lastLEDState == LOW) blinkCount--;
      if(!blinkCount){ flagBlink = 0; firstTime = true; }
    }
  }
}

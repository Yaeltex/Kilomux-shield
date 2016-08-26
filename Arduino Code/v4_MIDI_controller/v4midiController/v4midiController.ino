
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
#include <Kilomux.h>
#include <KilomuxDefs.h>
#include <KMS.h>
#include <KM_Accessors.h>
#include <KM_Data.h>
#include <KM_EEPROM.h>
#include <NewPing.h>
#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>
//#include <midi_Message.h>      // Si no compila porque le falta midi_message.h, descomentar esta linea

void setup(); // Esto es para solucionar el bug que tiene Arduino al usar los #ifdef del preprocesador

#define MIDI_COMMS


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Descomentar la próxima línea si el compilador no encuentra MIDI
MIDI_CREATE_DEFAULT_INSTANCE()
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


// AJUSTABLE - Si hay ruido que varía entre valores (+- 1, +- 2, +- 3...) colocar el umbral en (1, 2, 3...)
#define UMBRAL_RUIDO        1                      // Ventana de ruido para las entradas analógicas. Si entrada < entrada+umbral o 

#define ANALOGO_CRECIENDO     1
#define ANALOGO_DECRECIENDO   0

#define BLINK_INTERVAL 300

KMS::GlobalData gD = KMS::globalData();

Kilomux KMShield;                                       // Objeto de la clase Kilomux

static const char * const modeLabels[] = {
    "Off"
  , "Note"
  , "CC"
  , "NRPN"
  , "Shifter"
};
#define MODE_LABEL(m)   ((m) <= KMS::M_SHIFTER ? modeLabels[m] : "???")

// Contadores, flags //////////////////////////////////////////////////////////////////////
byte i, j, mux, canal;                       // Contadores para recorrer los multiplexores
byte numBanks, numInputs, numOutputs;
bool ledMode;
bool flagBlink = 0;                         
unsigned long millisPrevLED = 0;               // Variable para guardar ms
static byte blinkCount = 0;
///////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);                  // Se inicializa la comunicación serial.
  
  KMS::initialize();

  if(gD.isValid()){
    ledMode = gD.hasOutputMatrix();
    numBanks = gD.numBanks();
    numInputs = gD.numInputsNorm();
    numOutputs = gD.numOutputs();
    Serial.println("OK");
  }
  // Setear todos las lecturas a 0
  for (int mux = 0; mux < NUM_MUX; mux++) {                
    for (canal = 0; canal < NUM_MUX_CHANNELS; canal++) {
      KMShield.muxReadings[mux][canal] = 0;
      KMShield.muxPrevReadings[mux][canal] = 0;
    }
  }
}

void loop(){
  #ifdef MIDI_COMMS
  LeerMidi();
  #endif

  blinkLED();
  
  LeerEntradas();
}

/*
 * Esta función lee todas las entradas análógicas y/o digitales y almacena los valores de cada una en la matriz 'lecturas'.
 * Compara con los valores previos, almacenados en 'lecturasPrev', y si cambian, y llama a las funciones que envían datos.
 */
void LeerEntradas(void) {
  for(int contadorInput = 0; contadorInput < 32; contadorInput++){
    KMS::InputNorm input = KMS::input(contadorInput);
    mux = contadorInput/NUM_MUX_CHANNELS;
    canal = contadorInput%NUM_MUX_CHANNELS;
    
    if (input.mode() != KMS::M_OFF){
      if(input.analog() == KMS::T_ANALOG){
        KMShield.muxReadings[mux][canal] = KMShield.analogReadKm(mux, canal) >> 3;          // Leer entradas analógicas 'KMShield.analogReadKm(N_MUX,N_CANAL)'
                                                                                            // El valor leido va de 0-1023. Convertimos a 0-127, dividiendo por 8.
        if (!EsRuido(KMShield.muxReadings[mux][canal], KMShield.muxPrevReadings[mux][canal], canal, mux)) {                                              // Si lo que leo no es ruido
          SendInputMessage(KMShield.muxReadings[mux][canal], input);                                   // Envío datos SERIAL
        }
        else{
          continue;                                                                           // Sigo con la próxima lectura
        }
        KMShield.muxPrevReadings[mux][canal] = KMShield.muxReadings[mux][canal];             // Almacenar lectura actual como anterior, para el próximo ciclo                                                                                               
      }
    }
//      if(entradaActivada[mux][canal]){
//        if(tipoEntrada[mux][canal] == ANALOGICA){
//          // ENTRADAS ANALÓGICAS 1 ///////////////////////////////////////////////////
//          unsigned int analogData = KMShield.analogReadKm(mux, canal);          // Leer entradas analógicas 'KMShield.analogReadKm(N_MUX,N_CANAL)'
//          lecturas[mux][canal] = analogData >> 3;                                 // El valor leido va de 0-1023. Convertimos a 0-127, dividiendo por 8.
//  
//          if (!EsRuido(lecturas[mux][canal], lecturasPrev[mux][canal], canal, mux)) {                                              // Si lo que leo no es ruido
//            #if defined(COMUNICACION_MIDI)|defined(HAIRLESS_MIDI)
//              EnviarControlChangeMidi(lecturas[mux][canal], canal);                                     // Envío datos MIDI
//            #elif defined(COMUNICACION_SERIAL)
//              EnviarControlChangeSerial(lecturas[mux][canal], canal);                                   // Envío datos SERIAL
//            #endif
//          }
//          else{
//            continue;                                                          // Sigo con la próxima lectura
//          }
//        ///////////////////////////////////////////////////////////////////////////
//          lecturasPrev[mux][canal] = lecturas[mux][canal];             // Almacenar lectura actual como anterior, para el próximo ciclo  
//        }
//        // ENTRADAS DIGITALES /////////////////////////////////////////////////////
//        else if(tipoEntrada[mux][canal] == DIGITAL){
//          // CÓDIGO PARA LECTURA DE ENTRADAS DIGITALES Y SHIFTERS
//          lecturas[mux][canal] = KMShield.digitalReadKm(mux, canal, PULLUP);      // Leer entradas digitales 'KMShield.digitalReadKm(N_MUX, N_CANAL)'
//  
//          if (lecturas[mux][canal] != lecturasPrev[mux][canal]) {             // Me interesa la lectura, si cambió el estado del botón,
//            lecturasPrev[mux][canal] = lecturas[mux][canal];                  // Almacenar lectura actual como anterior, para el próximo ciclo
//            
//            if (!lecturas[mux][canal]){                                               // Si leo 0 (botón accionado)                                                                                                                                                        
//              #if defined(SHIFTERS)                                                   // Si estoy usando SHIFTERS                                                                                        
//              if (byte shift = EsShifter(canal)){
//                CambiarBanco(shift);
//                continue;   // Si es un shifter, seguir con la próxima lectura.
//              }
//              #endif    // endif SHIFTERS
//              
//              estadoBoton[bancoActivo][canal] = !estadoBoton[bancoActivo][canal];     // MODO TOGGLE: Cambia de 0 a 1, o viceversa
//                                                                                      // MODO NORMAL: Cambia de 0 a 1
//              // Se envía el estado del botón por MIDI o SERIAL
//              #if defined(COMUNICACION_MIDI)|defined(HAIRLESS_MIDI)
//              EnviarNoteMidi(canal, estadoBoton[bancoActivo][canal]*127);                                  // Envío MIDI
//              #elif defined(COMUNICACION_SERIAL)
//              EnviarNoteSerial(canal, estadoBoton[bancoActivo][canal]*127);                                // Envío SERIAL
//              estadoLeds[bancoActivo][mapeoLeds[mapeoNotes[canal]]] = estadoBoton[bancoActivo][canal]*2;   // Esta línea cambia el estado de los LEDs cuando se presionan los botones (solo SERIAL)
//              cambioEstadoLeds = 1;                                                                        // Actualizar LEDs
//              #endif              // endif COMUNICACION
//            }   
//                                                      
//            else if (lecturas[mux][canal]) {                        // Si se lee que el botón pasa de activo a inactivo (lectura -> 5V) y el estado previo era Activo
//              #if !defined(TOGGLE)
//              estadoBoton[bancoActivo][canal] = 0;                  // Se actualiza el flag a inactivo
//              #endif
//              
//              #if defined(SHIFTERS)                                 // Si estoy usando SHIFTERS      
//              // Si el botón presionado es uno de los SHIFTERS de bancos
//                if (EsShifter(canal)){
//                  #if !defined(TOGGLE_SHIFTERS)
//                  CambiarBanco(-1);  
//                  #endif
//                  continue;
//                }           
//              #endif       // endif SHIFTERS
//              #if !defined(TOGGLE)                                          // Si no estoy usando el modo TOGGLE
//                #if defined(COMUNICACION_MIDI)|defined(HAIRLESS_MIDI)    
//                EnviarNoteMidi(canal, NOTE_OFF);                      // Envío MIDI
//                #elif defined(COMUNICACION_SERIAL)              
//                EnviarNoteSerial(canal, NOTE_OFF);                    // Envío SERIAL
//                estadoLeds[bancoActivo][mapeoLeds[mapeoNotes[canal]]] = LED_APAGADO;
//                cambioEstadoLeds = 1;
//                #endif      // endif COMUNICACION                                      
//              #endif     // endif TOGGLE
//            } 
//          }
//        }
//      }
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

#ifdef MIDI_COMMS
// Lee el canal midi, note y velocity, y actualiza el estado de los leds.
void LeerMidi(void) {
  // COMENTAR PARA DEBUGGEAR CON SERIAL ///////////////////////////////////////////////////////////////////////
  if (MIDI.read()) {         // ¿Llegó un mensaje MIDI?
    switch (MIDI.getType())      // Identifica que tipo de mensaje llegó (NoteOff, NoteOn, AfterTouchPoly, ControlChange, ProgramChange, AfterTouchChannel, PitchBend, SystemExclusive)
    {
      case midi::SystemExclusive:
      {
        const byte *pMsg;
        byte *pMsg2;
        unsigned int sysexLength = 0;
        
        sysexLength = MIDI.getSysExArrayLength();
        pMsg = MIDI.getSysExArray();
        
        char sysexID[3];
        sysexID[0] = (char) pMsg[1]; 
        sysexID[1] = (char) pMsg[2]; 
        sysexID[2] = (char) pMsg[3];
        
        char command = pMsg[4];
        
        if(sysexID[0] == 'Y' && sysexID[1] == 'T' && sysexID[2] == 'X'){
          
          if (command == 1){            // Enter config mode
            for(int i=0; i<3;i++){
              flagBlink = 1;
              blinkCount = 1;
            }
          }                             
          
          if(command == 3){             // Save dump data
            KMS::io.write(200*pMsg[5], pMsg+6, sysexLength-7);  
            flagBlink = 1;
            blinkCount = 3;
          }  
        }
      }
      break;
      
      default:
        break;
    }
  }
  // FIN COMENTARIO ///////////////////////////////////////////////////////////////////////////////////////////
}
#endif

/*
 * Funcion para filtrar el ruido analógico de los pontenciómetros. Analiza si el valor crece o decrece, y en el caso de un cambio de dirección, 
 * decide si es ruido o no, si hubo un cambio superior al valor anterior más el umbral de ruido.
 * 
 * Recibe: - 
 */

unsigned int EsRuido(unsigned int valor, unsigned int valorPrev, unsigned int nota, unsigned int mux) {
  static bool estado[NUM_MUX_CHANNELS*2] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
                                            
  if (estado[nota] == ANALOGO_CRECIENDO){
    if(valor > valorPrev){              // Si el valor está creciendo, y la nueva lectura es mayor a la anterior, 
       return 0;                                                      // no es ruido.
    }
    else if(valor < valorPrev - UMBRAL_RUIDO){   // Si el valor está creciendo, y la nueva lectura menor a la anterior menos el UMBRAL
      estado[nota] = ANALOGO_DECRECIENDO;                                     // se cambia el estado a DECRECIENDO y 
      return 0;                                                               // no es ruido.
    }
  }
  if (estado[nota] == ANALOGO_DECRECIENDO){                                   
    if(valor < valorPrev){  // Si el valor está decreciendo, y la nueva lectura es menor a la anterior,  
       return 0;                                        // no es ruido.
    }
    else if(valor > valorPrev + UMBRAL_RUIDO){    // Si el valor está decreciendo, y la nueva lectura mayor a la anterior mas el UMBRAL
      estado[nota] = ANALOGO_CRECIENDO;                                       // se cambia el estado a CRECIENDO y 
      return 0;                                                               // no es ruido.
    }
  }
  return 1;         // Si todo lo anterior no se cumple, es ruido.
}

void SendInputMessage(unsigned int valor, const KMS::InputNorm &d) {
  Serial.print("Parametro: "); Serial.print(d.param_fine()); Serial.print("  Valor: "); Serial.println(valor);
  return; 
}

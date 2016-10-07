#include "Inputs.h"
#include "Comms.h"

/*
 * Esta función lee todas las entradas análógicas y/o digitales y almacena los valores de cada una en la matriz 'lecturas'.
 * Compara con los valores previos, almacenados en 'lecturasPrev', y si cambian, y llama a las funciones que envían datos.
 */
void ReadInputs() {
  for(int contadorInput = 0; contadorInput < numInputs; contadorInput++){
    KMS::InputNorm input = KMS::input(contadorInput);
    byte mux = contadorInput/NUM_MUX_CHANNELS;
    byte canal = contadorInput%NUM_MUX_CHANNELS;
    
    if (input.mode() != KMS::M_OFF){
      if(input.analog() == KMS::T_ANALOG){
        if(input.mode() == KMS::M_NRPN)
          KMShield.muxReadings[mux][canal] = KMShield.analogReadKm(mux, canal);          // Si es NRPN leer entradas analógicas 'KMShield.analogReadKm(N_MUX,N_CANAL)'
        else                                                                                 
          KMShield.muxReadings[mux][canal] = KMShield.analogReadKm(mux, canal) >> 3;     // Si no es NRPN, leer entradas analógicas 'KMShield.analogReadKm(N_MUX,N_CANAL)'
                                                                                         // El valor leido va de 0-1023. Convertimos a 0-127, dividiendo por 8.
                                                                                         
        if (!IsNoise(KMShield.muxReadings[mux][canal], KMShield.muxPrevReadings[mux][canal], contadorInput)) {  // Si lo que leo no es ruido
          InputChanged(input, KMShield.muxReadings[mux][canal]);                             // Envío datos
        }
        else{
          continue;                                                                          // Sigo con la próxima lectura
        }
        KMShield.muxPrevReadings[mux][canal] = KMShield.muxReadings[mux][canal];             // Almacenar lectura actual como anterior, para el próximo ciclo
      }
      
      else if(input.analog() == KMS::T_DIGITAL){
        // CÓDIGO PARA LECTURA DE ENTRADAS DIGITALES Y SHIFTERS
        KMShield.muxReadings[mux][canal] = KMShield.digitalReadKm(mux, canal, PULLUP);      // Leer entradas digitales 'KMShield.digitalReadKm(N_MUX, N_CANAL)'
        if (KMShield.muxReadings[mux][canal] != KMShield.muxPrevReadings[mux][canal]) {     // Me interesa la lectura, si cambió el estado del botón,
          KMShield.muxPrevReadings[mux][canal] = KMShield.muxReadings[mux][canal];             // Almacenar lectura actual como anterior, para el próximo ciclo
          if (!KMShield.muxReadings[mux][canal]){
            digitalInputState[canal+mux*NUM_MUX_CHANNELS] = !digitalInputState[canal+mux*NUM_MUX_CHANNELS];   // MODO TOGGLE: Cambia de 0 a 1, o viceversa
                                                                                                              // MODO NORMAL: Cambia de 0 a 1
            InputChanged(input, KMShield.muxReadings[mux][canal]);
          }
          else if (KMShield.muxReadings[mux][canal]) {
            digitalInputState[canal+mux*NUM_MUX_CHANNELS] = 0;
            InputChanged(input, KMShield.muxReadings[mux][canal]);
          }
        }
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
//              digitalInputState[bancoActivo][canal] = !digitalInputState[bancoActivo][canal];     // MODO TOGGLE: Cambia de 0 a 1, o viceversa
//                                                                                      // MODO NORMAL: Cambia de 0 a 1
//              // Se envía el estado del botón por MIDI o SERIAL
//              #if defined(COMUNICACION_MIDI)|defined(HAIRLESS_MIDI)
//              EnviarNoteMidi(canal, digitalInputState[bancoActivo][canal]*127);                                  // Envío MIDI
//              #elif defined(COMUNICACION_SERIAL)
//              EnviarNoteSerial(canal, digitalInputState[bancoActivo][canal]*127);                                // Envío SERIAL
//              estadoLeds[bancoActivo][mapeoLeds[mapeoNotes[canal]]] = digitalInputState[bancoActivo][canal]*2;   // Esta línea cambia el estado de los LEDs cuando se presionan los botones (solo SERIAL)
//              cambioEstadoLeds = 1;                                                                        // Actualizar LEDs
//              #endif              // endif COMUNICACION
//            }   
//                                                      
//            else if (lecturas[mux][canal]) {                        // Si se lee que el botón pasa de activo a inactivo (lectura -> 5V) y el estado previo era Activo
//              #if !defined(TOGGLE)
//              digitalInputState[bancoActivo][canal] = 0;                  // Se actualiza el flag a inactivo
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


void InputChanged(const KMS::InputNorm &input, unsigned int value) {
  byte currBank = KMS::bank();
  #if defined(MIDI_COMMS)
  switch(input.mode()){
    case KMS::M_NOTE:
      MIDI.sendNoteOn(input.param_fine()+numInputs*currBank, value, input.channel());
    break;
    case KMS::M_CC:
      MIDI.sendControlChange(input.param_fine()+numInputs*currBank, value, input.channel());
    break;
    case KMS::M_NRPN:
      MIDI.sendControlChange( 101, input.param_coarse(), input.channel());
      MIDI.sendControlChange( 100, input.param_fine(), input.channel());
      MIDI.sendControlChange( 6, (value >> 7) & 0x7F, input.channel());
      MIDI.sendControlChange( 38, (value & 0x7F), input.channel());
    break;
    case KMS::M_PROGRAM:
      MIDI.sendProgramChange( value, input.channel());
    break;
    case KMS::M_SHIFTER:
    break;
    default:
    break;
  }
  #else
  Serial.print("Channel: "); Serial.print(input.channel()); Serial.print("\t");
  Serial.print("Modo: "); Serial.print(MODE_LABEL(input.mode())); Serial.print("\t");
  Serial.print("Parameter: "); Serial.print((input.param_coarse() << 7) | input.param_fine()); Serial.print("  Valor: "); Serial.println(value);
  #endif
  return; 
}

/*
 * Funcion para filtrar el ruido analógico de los pontenciómetros. Analiza si el valor crece o decrece, y en el caso de un cambio de dirección, 
 * decide si es ruido o no, si hubo un cambio superior al valor anterior más el umbral de ruido.
 * 
 * Recibe: - 
 */

unsigned int IsNoise(unsigned int value, unsigned int prevValue, unsigned int input) {
  static bool upOrDown[NUM_MUX_CHANNELS*2] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
                                            
  if (upOrDown[input] == ANALOGO_UP){
    if(value > prevValue){              // Si el valor está creciendo, y la nueva lectura es mayor a la anterior, 
       return 0;                        // no es ruido.
    }
    else if(value < prevValue - NOISE_THRESHOLD){   // Si el valor está creciendo, y la nueva lectura menor a la anterior menos el UMBRAL
      upOrDown[input] = ANALOGO_DOWN;                                     // se cambia el estado a DECRECIENDO y 
      return 0;                                                               // no es ruido.
    }
  }
  if (upOrDown[input] == ANALOGO_DOWN){                                   
    if(value < prevValue){  // Si el valor está decreciendo, y la nueva lectura es menor a la anterior,  
       return 0;                                        // no es ruido.
    }
    else if(value > prevValue + NOISE_THRESHOLD){    // Si el valor está decreciendo, y la nueva lectura mayor a la anterior mas el UMBRAL
      upOrDown[input] = ANALOGO_UP;                                       // se cambia el estado a CRECIENDO y 
      return 0;                                                               // no es ruido.
    }
  }
  return 1;         // Si todo lo anterior no se cumple, es ruido.
}

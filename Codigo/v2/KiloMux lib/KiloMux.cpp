/**
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
*  - Jorge Crowe
*  - Lucas Leal
*  - Dimitri Diakopoulos
*/

//////////////////////////////////////////////////////////////////////////////////////////
// INICIALIZACIÓN ////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

void InitKiloMux(void) {
  // Setear pines de salida de los registro de desplazamiento 74HC595
  pinMode(latchPin, OUTPUT);                
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  
#if defined(CON_ULTRASONIDO)
  pinMode(PIN_LED_ACT_US, OUTPUT);         // LED indicador para el sensor de distancia
  pinMode(PIN_BOTON_ACT_US,INPUT_PULLUP);  // Botón de activación para el sensor de distancia
#endif  

#if defined(ENTRADA_DIGITAL)                                                    
  pinMode(ENTRADA_DIGITAL, INPUT);                // Estas dos líneas setean el pin analógico que recibe las entradas digitales como pin digital y
  digitalWrite(ENTRADA_DIGITAL, HIGH);            // setea el resistor de Pull-Up en el mismo
#endif // endif ENTRADA_DIGITAL

  // Guardar cantidad de ms desde el encendido
  anteriorMillis = millis();
  
  // Setear todos las lecturas a 0
  for (mux = 0; mux < NUM_MUX; mux++) {                
    for (canal = 0; canal < NUM_CANALES_MUX; canal++) {
      lecturas[mux][canal] = 0;
      lecturasPrev[mux][canal] = 0;
    }
    #if defined(MUX_DIGITAL)
      if (mux == MUX_DIGITAL) estadoBoton[bancoActivo][canal] = 0;     
    #endif // endif MUX_DIGITAL
  }
  // Setear todos los leds a 0
  for (i = 0; i < BANCOS; i++) {                
    for (j = 0; j < NUM_LEDS_X_BANCO; j++) {
       estadoLeds[i][j] = LED_APAGADO;
       estadoBoton[i][j] = 0;
    }
    estadoShifter[i] = 0;
  }
  
  // Ésta función setea la velocidad de muestreo del conversor analógico digital.
  SetADCprescaler(PS_16);

  LimpiarRegistros595();                             // Se limpia el array registros (todos a LOW)
  EscribirRegistros595();                             // Se envían los datos al 595

#if defined(COMUNICACION_MIDI)
  MIDI.begin(MIDI_CHANNEL_OMNI); MIDI.turnThruOff();  // Se inicializa la comunicación MIDI.
                                                      // Por default, la librería de Arduino MIDI tiene el THRU en ON, y NO QUEREMOS ESO!
#elif defined(COMUNICACION_SERIAL)
  Serial.begin(VELOCIDAD_SERIAL);                  // Se inicializa la comunicación serial. 
#elif defined(HAIRLESS_MIDI)
  MIDI.begin(MIDI_CHANNEL_OMNI); MIDI.turnThruOff();  // Se inicializa la comunicación MIDI.
                                                      // Por default, la librería de Arduino MIDI tiene el THRU en ON, y NO QUEREMOS ESO!
  Serial.begin(VELOCIDAD_SERIAL);                  // Se inicializa la comunicación serial. 
#endif
}
//////////////////////////////////////////////////////////////////////////////////////////
// PROGRAMA PRINCIPAL ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  // LECTURA DE DATOS POR CANAL MIDI O SERIAL //////////////////////////////
  #if defined(COMUNICACION_MIDI)|defined(HAIRLESS_MIDI)       
  LeerMidi();                          
  #elif defined(COMUNICACION_SERIAL)
  EncenderLedsSerial();
  #endif    //endif COMUNICACION 

  // LECTURA DE ENTRADAS ///////////////////////////////////////////////////
  // Esta función recorre las entradas de los multiplexores y envía datos MIDI si detecta cambios.  
  LeerEntradas();
  
  #if defined(CON_ULTRASONIDO)
  // Función que lee los datos que reporta el sensor ultrasónico, filtra y envía la información MIDI si hay cambios.
  LeerUltrasonico();
  #endif

  // ACTUALIZACIÓN DE SALIDAS //////////////////////////////////////////////
  if(cambioBanco || cambioEstadoLeds){
    ActualizarLeds();
    cambioBanco = 0; cambioEstadoLeds = 0;  
  }
  if (millis() - anteriorMillis > INTERVALO_LEDS)          // Si transcurrieron más de X ms desde la ultima actualización,
    TitilarLeds(); 
  ////////////////////////////////////////////////////////////////////////// 
}

/*
 * Esta función lee todas las entradas análógicas y/o digitales y almacena los valores de cada una en la matriz 'lecturas'.
 * Compara con los valores previos, almacenados en 'lecturasPrev', y si cambian, y llama a las funciones que envían datos.
 */
void LeerEntradas(void) {
  for (mux = 0; mux < NUM_MUX; mux++) {                                   // Recorro el numero de multiplexores, definido al principio
    for (canal = 0; canal < NUM_CANALES_MUX; canal++) {                   // Recorro todos los canales de cada multiplexor
      // ENTRADAS ANALÓGICA 1 ///////////////////////////////////////////////////
      if (mux == MUX_ANALOGICO) {                                         // Si es un potenciómetro/fader/sensor,
        unsigned int analogData = muxShield.analogReadMS(mux + 1, canal); // Leer entradas analógicas 'muxShield.analogReadMS(N_MUX,N_CANAL)'
        lecturas[mux][canal] = analogData >> 3;                           // El valor leido va de 0-1023. Convertimos a 0-127, dividiendo por 8.
        
        if (EsRuido(canal) == 0) {                                        // Si lo que leo no es ruido
          #if defined(COMUNICACION_MIDI)|defined(HAIRLESS_MIDI)
            EnviarControlChangeMidi(canal);                               // Envío datos MIDI
          #elif defined(COMUNICACION_SERIAL)
            EnviarControlChangeSerial(canal);                             // Envío datos SERIAL
          #endif
        }
        else continue;                                                    // Sigo con la próxima lectura
      }
      ///////////////////////////////////////////////////////////////////////////
      
      // ENTRADA ANALÓGICA 2 - SI EXISTE ////////////////////////////////////////
      #if defined(MUX_ANALOGICO_2)      
      // ENTRADAS ANALÓGICAS 
      if (mux == MUX_ANALOGICO_2) {                                        // Si es un potenciómetro/fader/sensor,
        // CÓDIGO PARA LECTURA DE ENTRADAS ANALÓGICAS /////////////////////////////////////////////////////////////////////
        unsigned int analogData = muxShield.analogReadMS(mux + 1, canal);  // Leer entradas analógicas 'muxShield.analogReadMS(N_MUX,N_CANAL)'
        lecturas[mux][canal] = analogData >> 3;                            // El valor leido va de 0-1023. Convertimos a 0-127, dividiendo por 8.
        
        if (EsRuido(canal+8) == 0) {                                       // Si lo que leo no es ruido
          #if defined(COMUNICACION_MIDI)|defined(HAIRLESS_MIDI)
            EnviarControlChangeMidi(canal+8);               
          #elif defined(COMUNICACION_SERIAL)
            EnviarControlChangeSerial(canal+8);
          #endif
        }
        else continue;
      }
      #endif      // endif MUX_ANALOGICO_2
      ///////////////////////////////////////////////////////////////////////////
       
      // ENTRADAS DIGITALES /////////////////////////////////////////////////////
      #if defined(MUX_DIGITAL)
      if (mux == MUX_DIGITAL) {                                             // Si es un pulsador,
        // CÓDIGO PARA LECTURA DE ENTRADAS DIGITALES 
        lecturas[mux][canal] = muxShield.digitalReadMS(mux + 1, canal);     // Leer entradas digitales 'muxShield.digitalReadMS(N_MUX, N_CANAL)'
        
        if (lecturas[mux][canal] != lecturasPrev[mux][canal]) {             // Me interesa la lectura, si cambió el estado del botón,
          lecturasPrev[mux][canal] = lecturas[mux][canal];                  // Almacenar lectura actual como anterior, para el próximo ciclo
          
          if (!lecturas[mux][canal]){                                               // Si leo 0 (botón accionado)                                                                        
                                                                                                
            #if defined(SHIFTERS)                                                   // Si estoy usando SHIFTERS                                                                                        
            if (byte shift = EsShifter(canal)){
              CambiarBanco(shift);
              continue;   // Si es un shifter, seguir con la próxima lectura.
            }
            #endif    // endif SHIFTERS
            
            estadoBoton[bancoActivo][canal] = !estadoBoton[bancoActivo][canal];     // MODO TOGGLE: Cambia de 0 a 1, o viceversa
                                                                                    // MODO NORMAL: Cambia de 0 a 1
            // Se envía el estado del botón por MIDI o SERIAL
            #if defined(COMUNICACION_MIDI)|defined(HAIRLESS_MIDI)
            EnviarNoteMidi(canal, estadoBoton[bancoActivo][canal]*127);                                  // Envío MIDI
            #elif defined(COMUNICACION_SERIAL)
            EnviarNoteSerial(canal, estadoBoton[bancoActivo][canal]*127);                                // Envío SERIAL
            estadoLeds[bancoActivo][mapeoLeds[mapeoNotes[canal]]] = estadoBoton[bancoActivo][canal]*2;   // Esta línea cambia el estado de los LEDs cuando se presionan los botones (solo SERIAL)
            cambioEstadoLeds = 1;                                                                        // Actualizar LEDs
            #endif              // endif COMUNICACION
          }   
                                                    
          else if (lecturas[mux][canal]) {                        // Si se lee que el botón pasa de activo a inactivo (lectura -> 5V) y el estado previo era Activo
            #if !defined(TOGGLE)
            estadoBoton[bancoActivo][canal] = 0;                  // Se actualiza el flag a inactivo
            #endif
            
            #if defined(SHIFTERS)                                 // Si estoy usando SHIFTERS      
            // Si el botón presionado es uno de los SHIFTERS de bancos
              if (EsShifter(canal)){
                #if !defined(TOGGLE_SHIFTERS)
                CambiarBanco(-1);  
                #endif
                continue;
              }           
            #endif       // endif SHIFTERS
            #if !defined(TOGGLE)                                          // Si no estoy usando el modo TOGGLE
              #if defined(COMUNICACION_MIDI)|defined(HAIRLESS_MIDI)    
              EnviarNoteMidi(canal, NOTE_OFF);                      // Envío MIDI
              #elif defined(COMUNICACION_SERIAL)              
              EnviarNoteSerial(canal, NOTE_OFF);                    // Envío SERIAL
              estadoLeds[bancoActivo][mapeoLeds[mapeoNotes[canal]]] = LED_APAGADO;
              cambioEstadoLeds = 1;
              #endif      // endif COMUNICACION                                      
            #endif     // endif TOGGLE
          } 
          
        }
        ///////////////////////////////////////////////////////////////////////////
        continue;
      }
      #endif  // endif MUX_DIGITAL
      lecturasPrev[mux][canal] = lecturas[mux][canal];             // Almacenar lectura actual como anterior, para el próximo ciclo
    }
  }
}

#if defined(SHIFTERS)                                           
byte EsShifter(unsigned int nBoton){
  if(mapeoNotes[nBoton] == BOTON_BANCO_1){                        
    return 1;
  }
  #if defined(BOTON_BANCO_2)
  else if(mapeoNotes[nBoton] == BOTON_BANCO_2){
    return 2;
  }
  #endif
  #if defined(BOTON_BANCO_3)
  else if(mapeoNotes[nBoton] == BOTON_BANCO_3){
    return 3;
  }
  #endif
  #if defined(BOTON_BANCO_4)
  else if(mapeoNotes[nBoton] == BOTON_BANCO_4){
    return 4;
  }
  #endif
  else
    return 0;    // si no, devuelvo FALSE
}

void CambiarBanco(int shifter){  
  if (shifter < 0){
    estadoShifter[SHIFTER_1] = OFF;
    estadoShifter[SHIFTER_2] = OFF;
    estadoShifter[SHIFTER_3] = OFF;
    estadoShifter[SHIFTER_4] = OFF;
    bancoActivo = 0; cambioBanco = 1;
    return;
  }
 
  // En caso que el botón presionado sea uno de los shifters, se cambia el banco activo y se señala con el flag que se deben actualizar los LEDs
  switch(bancoActivo){
    case BANCO_0:
      #if defined(COMUNICACION_SERIAL)
      Serial.print("Banco "); Serial.println(shifter); 
      #endif
      bancoActivo = shifter; estadoShifter[shifter-1] = ON;
    break;
    #if defined(BANCO_1)
    case BANCO_1:
      if (shifter == SHIFTER_1+1 && estadoShifter[SHIFTER_1] == ON){
        #if defined(COMUNICACION_SERIAL)
        Serial.println("Banco 0");
        #endif
        bancoActivo = 0; estadoShifter[SHIFTER_1] = OFF;
      }
      else{
        #if defined(COMUNICACION_SERIAL)
        Serial.print("Banco "); Serial.println(shifter); 
        #endif
        bancoActivo = shifter; estadoShifter[shifter-1] = ON;
                               estadoShifter[SHIFTER_1] = OFF;
      }
    break;
    #endif
    #if defined(BANCO_2)
    case BANCO_2:
      if (shifter == SHIFTER_2+1 && estadoShifter[SHIFTER_2] == ON){
        #if defined(COMUNICACION_SERIAL)
        Serial.println("Banco 0");
        #endif
        bancoActivo = 0; estadoShifter[SHIFTER_2] = OFF;
      }
      else{
        #if defined(COMUNICACION_SERIAL)
        Serial.print("Banco "); Serial.println(shifter); 
        #endif
        bancoActivo = shifter; estadoShifter[shifter-1] = ON;
                               estadoShifter[SHIFTER_2] = OFF;
      }
    break;
    #endif
    #if defined(BANCO_3)
    case BANCO_3:
      if (shifter == SHIFTER_3+1 && estadoShifter[SHIFTER_3] == ON){
        #if defined(COMUNICACION_SERIAL)
        Serial.println("Banco 0");
        #endif
        bancoActivo = 0; estadoShifter[SHIFTER_3] = OFF;
      }
      else{
        #if defined(COMUNICACION_SERIAL)
        Serial.print("Banco "); Serial.println(shifter); 
        #endif
        bancoActivo = shifter; estadoShifter[shifter-1] = ON;
                               estadoShifter[SHIFTER_3] = OFF;
      }
    break;
    #endif
    #if defined(BANCO_4)
    case BANCO_4:
      if (shifter == SHIFTER_4+1 && estadoShifter[SHIFTER_4] == ON){
        #if defined(COMUNICACION_SERIAL)
        Serial.println("Banco 0");
        #endif
        bancoActivo = 0; estadoShifter[SHIFTER_4] = OFF;
      }
      else{
        #if defined(COMUNICACION_SERIAL)
        Serial.print("Banco "); Serial.println(shifter); 
        #endif
        bancoActivo = shifter; estadoShifter[shifter-1] = ON;
                               estadoShifter[SHIFTER_4] = OFF;
      }
    break;
    #endif
    default:
    break;
  }  
  cambioBanco = 1; 
  return;
}
#endif            // endif SHIFTERS
/*
 * Funcion para filtrar el ruido analógico de los pontenciómetros. Analiza si el valor crece o decrece, y en el caso de un cambio de dirección, 
 * decide si es ruido o no, si hubo un cambio superior al valor anterior más el umbral de ruido.
 * 
 * Recibe: - 
 */

unsigned int EsRuido(unsigned int nota) {
  #if !defined(MUX_ANALOGICO_2)
  static bool estado[NUM_CANALES_MUX] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  #else
  static bool estado[NUM_CANALES_MUX*2] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  #endif
  static unsigned int valorPrev = 0;
  if (estado[nota] == ANALOGO_CRECIENDO){
    if(lecturas[mux][canal] > lecturasPrev[mux][canal]){              // Si el valor está creciendo, y la nueva lectura es mayor a la anterior, 
       return 0;                                                      // no es ruido.
    }
    else if(lecturas[mux][canal] < lecturasPrev[mux][canal] - UMBRAL_RUIDO){   // Si el valor está creciendo, y la nueva lectura menor a la anterior menos el UMBRAL
      estado[nota] = ANALOGO_DECRECIENDO;                                     // se cambia el estado a DECRECIENDO y 
      return 0;                                                               // no es ruido.
    }
  }
  if (estado[nota] == ANALOGO_DECRECIENDO){                                   
    if(lecturas[mux][canal] < lecturasPrev[mux][canal]){  // Si el valor está decreciendo, y la nueva lectura es menor a la anterior,  
       return 0;                                        // no es ruido.
    }
    else if(lecturas[mux][canal] > lecturasPrev[mux][canal] + UMBRAL_RUIDO){    // Si el valor está decreciendo, y la nueva lectura mayor a la anterior mas el UMBRAL  
      estado[nota] = ANALOGO_CRECIENDO;                                       // se cambia el estado a CRECIENDO y 
      return 0;                                                               // no es ruido.
    }
  }
  return 1;         // Si todo lo anterior no se cumple, es ruido.
}

#if defined(CON_ULTRASONIDO)
void LeerUltrasonico(void){
  static unsigned int antMillisUltraSonico = 0;    // Variable usada para almacenar los milisegundos desde el último Ping al sensor
  static bool estadoBotonUSAnt = 1;                // Inicializo inactivo (entrada activa baja)
  static bool estadoBotonUS = 1;                   // Inicializo inactivo (entrada activa baja)
  static bool estadoLed = 0;                       // Inicializo inactivo (salida activa alta)
  static bool sensorActivado = 0;                  // Inicializo inactivo (variable interna)
  static unsigned int indiceFiltro = 0;            // Indice que recorre los valores del filtro de suavizado
    
  int microSeg = 0;                                // Contador de microsegundos para el ping del sensor.
  
  // Este codigo verifica si se presionó el botón y activa o desactiva el sensor cada vez que se presiona
  estadoBotonUS = digitalRead(PIN_BOTON_ACT_US);
  if(estadoBotonUS == LOW && estadoBotonUSAnt == HIGH){    // Si el botón previamente estaba en estado alto, y ahora esta en estado bajo, quiere decir que paso de estar no presionado a presionado (activo bajo)
    estadoBotonUSAnt = LOW;                                // Actualizo el estado previo
    sensorActivado = !sensorActivado;                      // Activo o desactivo el sensor
    estadoLed = !estadoLed;                                // Cambio el estado del LED
    digitalWrite(PIN_LED_ACT_US, estadoLed);               // Y actualizo la salida
  }
  else if(estadoBotonUS == HIGH && estadoBotonUSAnt == LOW){  // Si el botón previamente estaba en estado bajo, y ahora esta en estado alto, quiere decir que paso de estar presionado a no presionado 
    estadoBotonUSAnt = HIGH;                                  // Actualizo el estado previo
  } 
  
  if(sensorActivado){                                       // Si el sensor está activado
    if (millis()-antMillisUltraSonico > DELAY_ULTRAS){      // y transcurrió el delay minimo entre lecturas
      antMillisUltraSonico = millis();                      
      microSeg = sensorUS.ping();                           // Sensar el tiempo que tarda el pulso de ultrasonido en volver. Se recibe el valor el us.
      int ccSensorValue = map(constrain(microSeg, MIN_US, MAX_US), MIN_US, MAX_US, 0, 130);
      
      if(ccSensorValue != ccSensorValuePrev[indiceFiltro]){             // Si el valor actual es distinto al anterior, filtro y envío datos
        if(!ccSensorValue & ccSensorValuePrev[indiceFiltro] > 10){      // Este if no permite que si se saca la mano o se excede la distancia maxima, el valor vuelva a 0
          ccSensorValue =  ccSensorValuePrev[indiceFiltro];             // igualando el valor actual con el último valor anterior, si el nuevo valor es 0 y el anterior es mayor a 10
        }
        
        // FILTRO DE MEDIA MÓVIL PARA SUAVIZAR LA LECTURA
        for(int i = 0; i < FILTRO_US; i++){                      
          ccSensorValue += ccSensorValuePrev[i];           // Suma al valor actual, los N (FILTRO_US) valores anteriores 
        }
        ccSensorValue /= FILTRO_US+1;                      // Divido por N+1
        ccSensorValuePrev[indiceFiltro++] = ccSensorValue; // Y actualizo el índice del buffer circular
        indiceFiltro %= FILTRO_US+1;
        // FIN FILTRADO ////////////////////////////////
        
        // Detecto si cambió el valor filtrado, para no mandar valores repetidos
        if(ccSensorValue != ccSensorValuePrev[indiceFiltro]){        
          #if defined(COMUNICACION_MIDI)|defined(HAIRLESS_MIDI)
            MIDI.sendControlChange(CC_ULTRASONIDO, ccSensorValue, CANAL_MIDI_CC);          // Envío MIDI
          #elif defined(COMUNICACION_SERIAL)  
            Serial.print("Sensor Ultrasonico CC: "); Serial.print(CC_ULTRASONIDO); Serial.print("  Valor: "); Serial.println(ccSensorValue);    // Envío SERIAL
          #endif
        }
      }
      else return;        
    }
  }
}
#endif    // endif CON_ULTRASONIDO

void ActualizarLeds(void){        
  static unsigned long int anteriorMillis = 0;
  unsigned int led = 0;
  for (led = 0; led < NUM_LEDS_X_BANCO; led++){
    if(estadoLeds[bancoActivo][led] == LED_ENCENDIDO){
      SetLed595(led, HIGH);
    }
    else if(estadoLeds[bancoActivo][led] == LED_APAGADO){
      SetLed595(led, LOW);
    }
  }
  #if defined(SHIFTERS)
  SetLed595(mapeoLeds[LED_SHIFTER_1],estadoShifter[SHIFTER_1]);
  SetLed595(mapeoLeds[LED_SHIFTER_2],estadoShifter[SHIFTER_2]);
  SetLed595(mapeoLeds[LED_SHIFTER_3],estadoShifter[SHIFTER_3]);
  SetLed595(mapeoLeds[LED_SHIFTER_4],estadoShifter[SHIFTER_4]); 
  #endif    // endif SHIFTERS
}

// Esta función actualiza el estado de los LEDs que titilan
void TitilarLeds(void) {
  unsigned int led = 0;
  for (led = 0; led < NUM_LEDS_X_BANCO; led++) {                   // Recorrer todos los LEDs
    if (estadoLeds[bancoActivo][led] == LED_TITILANDO) {     // Si corresponde titilar este LED,
      if (tiempo_on) {                                   // Y estaba encendido,
        SetLed595(led, HIGH);                               // Se apaga
      }
      else {                                           // Si estaba apagado,
        SetLed595(led, LOW);                              // Se enciende
      }
    }
  }
  anteriorMillis = millis();                     // Actualizo el contador de ms usado en la comparación
  tiempo_on = !tiempo_on;
}

#if defined(COMUNICACION_MIDI)|defined(HAIRLESS_MIDI)
// Lee el canal midi, note y velocity, y actualiza el estado de los leds.
void LeerMidi(void) {
  // COMENTAR PARA DEBUGGEAR CON SERIAL ///////////////////////////////////////////////////////////////////////
  if (MIDI.read(CANAL_MIDI_LEDS)) {         // ¿Llegó un mensaje MIDI?
    byte led_number = MIDI.getData1();  // Capturo la nota
    byte led_vel = MIDI.getData2();
    byte banco = led_number/16;
    led_number %= 16;
    led_number = mapeoLeds[led_number];
    switch (MIDI.getType())                  // Identifica que tipo de mensaje llegó (NoteOff, NoteOn, AfterTouchPoly, ControlChange, ProgramChange, AfterTouchChannel, PitchBend, SystemExclusive)
    {
      #if !defined(BOTONES_CC)
      case midi::NoteOn:                       // En caso que sea NoteOn,
      #else
      case midi::ControlChange:                // En caso que sea Control change,
      #endif
        if (led_vel > VELOCITY_LIM_TITILA) {     // Si la velocity es un valor entre LIMITE y 127, el led no parpadea
          estadoLeds[banco][led_number] = LED_ENCENDIDO;
          cambioEstadoLeds = 1;
        }
        else if (led_vel > 0 && led_vel <= VELOCITY_LIM_TITILA){     // Si la velocity es mayor a cero, y menor a LIMITE,
          estadoLeds[banco][led_number] = LED_TITILANDO;
          cambioEstadoLeds = 1;     // Flag para actualizar leds
        }  
        else {                                                      // Si las anteriores no se cumplen, entonces es velocity = 0, es decir NoteOff,
          estadoLeds[banco][led_number] = LED_APAGADO;
          cambioEstadoLeds = 1;     // Flag para actualizar leds
        }
      break;
      case midi::NoteOff:        // Apago también los LEDs si recibo cualquier NOTE-OFF
        estadoLeds[banco][led_number] = LED_APAGADO;  
        cambioEstadoLeds = 1;       // Flag para actualizar leds
      break;
      default:
        break;
    }
  }
  // FIN COMENTARIO ///////////////////////////////////////////////////////////////////////////////////////////
}
#endif  // endif COMUNICACION

#if defined(COMUNICACION_SERIAL)
void EncenderLedsSerial(void){
// EL DEBUG FUNCIONA ENVIANDO DESDE EL TERMINAL SERIE UN NUMERO DEL 0-9 O UNA LETRA DE a-f O DE A-F (10-15) QUE CORRESPONDE AL LED QUE QUEREMOS QUE CAMBIE DE ESTADO
  // CADA VEZ QUE LLEGA EL NUMERO, SE CAMBIA DE UN ESTADO AL SIGUIENTE
  // APAGADO -> TITILANDO -> ENCENDIDO -> APAGADO

  // DESCOMENTAR PARA DEBUGGEAR CON SERIAL ///////////////////////////////////////////////////////////////////////
    static unsigned int leds[BANCOS][NUM_LEDS_X_BANCO];
    if (Serial.available()){                // ¿Llegó un mensaje Serial?
      char inputBuffer[2] = {' ',' '};
      short int led_number = 0;  // Capturo la nota

      Serial.readBytes(inputBuffer, 2);
      led_number = atoi(inputBuffer);
      
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Si se recibe 99 por el SERIAL, se encienden todos los LEDs durante 1 segundo. 
      // Es útil para saber si funciona o no cada LED.
      if(led_number == 99){                     // Si llegó 99 por el SERIAL
          for (int i = 0; i<NUM_LEDS_X_BANCO; i++){     // Se inicia un ciclor "for" para recorrer todos los LEDs
            SetLed595(i, HIGH);                // Se enciende cada LED
          }
          delay(1000);                          // Delay de 1000mS 
          for (int i = 0; i<NUM_LEDS_X_BANCO; i++){     // Se inicia un ciclor "for" para recorrer todos los LEDs
            SetLed595(i, LOW);                 // Se apaga cada LED
          }
          return;                               // Se retorna de la función EncenderLedsSerial()
      }
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////
      byte banco = led_number/16;
      led_number %= 16;
      
      byte ledAEncender = mapeoLeds[led_number];
      
      leds[banco][ledAEncender] = (!leds[banco][ledAEncender])*2;
      if (leds[banco][ledAEncender] == LED_APAGADO){
        estadoLeds[banco][ledAEncender] = LED_APAGADO;
        #if defined(SHIFTERS)
        Serial.print("LED "); Serial.print(banco*16 + led_number); Serial.println(" apagado.");
        #else
        Serial.print("LED "); Serial.print(led_number%16); Serial.println(" apagado.");
        #endif
      }
      else if (leds[banco][ledAEncender] == LED_TITILANDO){
        estadoLeds[banco][ledAEncender] = LED_TITILANDO;
        #if defined(SHIFTERS)
        Serial.print("LED "); Serial.print(banco*16 + led_number); Serial.println(" titilando.");
        #else
        Serial.print("LED "); Serial.print(led_number%16); Serial.println(" titilando.");
        #endif
      }
      else if (leds[banco][ledAEncender] == LED_ENCENDIDO){
        estadoLeds[banco][ledAEncender] = LED_ENCENDIDO;
        #if defined(SHIFTERS)
        Serial.print("LED "); Serial.print(banco*16 + led_number); Serial.println(" encendido.");
        #else
        Serial.print("LED "); Serial.print(led_number%16); Serial.println(" encendido.");
        #endif
      }
      cambioEstadoLeds = 1;       // Flag para actualizar leds
    }
  // FIN COMENTARIO DEBUG ////////////////////////////////////o///////////////////////////////////////////////////////
}
#endif


#if defined(COMUNICACION_MIDI)|defined(HAIRLESS_MIDI)
void EnviarNoteMidi(unsigned int nota, unsigned int veloc) {
  #ifndef BOTONES_CC
  MIDI.sendNoteOn(mapeoNotes[nota]+SALTO_SHIFTER*bancoActivo, veloc, CANAL_MIDI_NOTES);
  #else
  MIDI.sendControlChange(mapeoNotes[nota]+SALTO_SHIFTER*bancoActivo, veloc, CANAL_MIDI_CC);
  #endif  // endif BOTONES_CC
  return;
}
#endif  // endif COMUNICACION

#if defined(COMUNICACION_SERIAL)
void EnviarNoteSerial(unsigned int nota, unsigned int veloc) {
// FIN COMENTARIO ///////////////////////////////////////////////////////////////////////////////////////////
  // DEBUG ///////////////////////////////////////////////////////////////////////////////////////////////////////////
  veloc /= 127;
  Serial.print("BOTON NOTE ON"); Serial.print("  Nota: "); Serial.print(mapeoNotes[nota]+SALTO_SHIFTER*bancoActivo); Serial.print("  Velocity: "); Serial.println(veloc);
}  
#endif  // endif COMUNICACION_SERIAL

#if defined(COMUNICACION_MIDI)|defined(HAIRLESS_MIDI)
// Remapea las entradas analógicas y las envía por MIDI
void EnviarControlChangeMidi(unsigned int nota) {
  MIDI.sendControlChange(mapeoCC[nota]+SALTO_SHIFTER*bancoActivo, lecturas[mux][canal], CANAL_MIDI_CC);
  return;
}
#endif  // endif COMUNICACION_SERIAL

#if defined(COMUNICACION_SERIAL)
void EnviarControlChangeSerial(unsigned int nota) {
  Serial.print("Numero de pote: "); Serial.print(mapeoCC[nota]+SALTO_SHIFTER*bancoActivo); Serial.print("  Valor: "); Serial.println(lecturas[mux][canal]);
  return; 
}
#endif  // endif COMUNICACION_SERIAL

// Función que define el Prescaler del ADC para cambiar la frecuencia de sampleo de las señales analógicas
void SetADCprescaler(byte setting){
    // Setear el prescaler (divisor de clock) del ADC
  ADCSRA &= ~PS_128;  // Poner a 0 los bits del registro seteados por la librería de Arduino
  // Elegimos uno de los prescalers:
  // PS_16 (1MHz o 50000 muestras/s) 
  // PS_32 (500KHz o 31250 muestras/s) 
  // PS_64 (250KHz o 16666 muestras/s)
  // PS_128 (125KHz o 8620 muestras/s)
  // Atmel sugiere mantener la frecuencia de operación del ADC entre 50Khz y 200Khz, advirtiendo que 
  // puede degradarse la resolución si se supera la misma
  ADCSRA |= setting;    // Setear el prescaler al valor elegido
}

// Prender o apagar un solo led mediante el 
void SetLed595(int num_led, int estado) {
  registros[num_led] = estado;
  EscribirRegistros595();
}

// Apagar todos los LEDs
void LimpiarRegistros595() {
  for (int i = NUM_LEDS_X_BANCO - 1; i >=  0; i--) {
    registros[i] = LOW;
  }
}

// Actualizar el registro de desplazamiento. Actualiza el estado de los LEDs
// Antes de llamar a ésta función, el array 'registros' debería contener el estado requerido para TODOS los LEDs
void EscribirRegistros595() {
  digitalWrite(latchPin, LOW);                  // Se baja la línea de latch para avisar al 595 que se van a enviar datos

  for (int i = NUM_LEDS_X_BANCO - 1; i >=  0; i--) {    // Se recorren todos los LEDs,
    digitalWrite(clockPin, LOW);                  // Se baja "manualmente" la línea de clock

    int val = registros[i];                       // Se recupera el estado del LED

    digitalWrite(dataPin, val);                   // Se escribe el estado del LED en la línea de datos
    digitalWrite(clockPin, HIGH);                 // Se levanta la línea de clock para el próximo bit
  }
  digitalWrite(latchPin, HIGH);                 // Se vuelve a poner en HIGH la línea de latch para avisar que no se enviarán mas datos
}
/*
Autor: Franco Grassano - YAELTEX
---
INFORMACIÓN DE LICENCIA
Kilo Mux Shield por Yaeltex se distribuye bajo una licencia
Creative Commons Atribución-CompartirIgual 4.0 Internacional - http://creativecommons.org/licenses/by-sa/4.0/
----
Código para el manejo de los integrados 74HC595 tomado de http://bildr.org/2011/02/74HC595/ - Licencia MIT (http://opensource.org/licenses/mit-license.php)
Librería de multiplexado tomada de http://mayhewlabs.com/products/mux-shield-2 
El firmware de HIDUINO fue tomado de https://github.com/ddiakopoulos/hiduino - Desarrollado por Dimitri Diakopulos y liberado bajo la licencia del MIT (http://opensource.org/licenses/mit-license.php)

Este código fue desarrollado para el KILO MUX SHIELD (desde v3.6 hasta ---) de Yaeltex, apuntando al desarrollo de controladores MIDI.
Está preparado para manejar 2 registros de desplazamiento 74HC595 conectados en cadena (16 salidas digitales en total),
y 2 multiplexores CD4067 de 16 canales cada uno (16 entradas analógicas, y 16 entradas digitales), pero es expandible en el
caso de utilizar hardware diferente. Para ello se modifican los "define" NUM_MUX, NUM_CANALES_MUX y NUM_595s.

Para las entradas analógicas, por cuestiones de ruido se recomienda usar potenciómetros o sensores con buena estabilidad, y en
lo posible con valores cercanos o menores a 10 Kohm.

Para debuggear con Serial se deben comentar las lineas 
216 - MIDI.begin(MIDI_CHANNEL_OMNI;
222 - leer_MIDI();
428 - enviarControlChangeMidi(nota);
445 - enviarNoteMidi(nota, NOTE_ON);
450 - enviarNoteMidi(nota, NOTE_OFF);

Y se deben sacar los comentarios de las lineas 
217 - Serial.begin(VELOCIDAD_SERIAL);       // Se inicializa la comunicación serial.
223 - encender_leds_serial();
429 - enviarControlChangeSerial(nota);  
446 - enviarNoteSerial(nota, NOTE_ON);  
451 - enviarNoteSerial(nota, NOTE_ON);  
*/

#include <MuxShield.h>
#include <MIDI.h>
#include <NewPing.h>
// Descomentar la próxima línea si el compilador no encuentra MIDI
// MIDI_CREATE_DEFAULT_INSTANCE();

#define NUM_595s 2                                 // Número de integrados 74HC595
#define NUM_LEDS NUM_595s * 8                      // Número de LEDs en total, NO CAMBIAR

#define NUM_MUX 2                                  // Número de multiplexores a direccionar
#define NUM_CANALES_MUX 16                         // Número de canales en cada mux

#define MUX_A 0
#define MUX_B 1
#define MUX_A_ENTRADA A0                           // Pin analógico por dónde se lee el MUX A
#define MUX_B_ENTRADA A1                           // Pin analógico por dónde se lee el MUX B

#define MUX_DIGITAL   MUX_A                        // CAMBIAR SEGÚN APLICACIÓN
#define MUX_ANALOGICO MUX_B                        // CAMBIAR SEGÚN APLICACIÓN

#define ENTRADA_DIGITAL   MUX_B_ENTRADA            // CAMBIAR SEGÚN APLICACIÓN -  Multiplexor con entradas digitales
#define ENTRADA_ANALOGICA MUX_A_ENTRADA            // CAMBIAR SEGÚN APLICACIÓN - Multiplexor con entradas analógicas

// Estos valores dependen de como están cableados los botones en el controlador
// FIJOS - NO TOCAR!!!
#define NOTA_0_HW  0
#define NOTA_1_HW  1
#define NOTA_2_HW  2
#define NOTA_3_HW  3
#define NOTA_4_HW  4
#define NOTA_5_HW  5
#define NOTA_6_HW  6
#define NOTA_7_HW  7
#define NOTA_8_HW  8
#define NOTA_9_HW  9
#define NOTA_10_HW 10
#define NOTA_11_HW 11
#define NOTA_12_HW 12
#define NOTA_13_HW 13
#define NOTA_14_HW 14
#define NOTA_15_HW 15

// Estos son los valores de notas que se quieren recibir en la aplicación DAW.
// AJUSTABLES SEGÚN APLICACIÓN
#define NOTA_0_MIDI  0
#define NOTA_1_MIDI  1
#define NOTA_2_MIDI  2  
#define NOTA_3_MIDI  3
#define NOTA_4_MIDI  4
#define NOTA_5_MIDI  5
#define NOTA_6_MIDI  6
#define NOTA_7_MIDI  7
#define NOTA_8_MIDI  8
#define NOTA_9_MIDI  9
#define NOTA_10_MIDI 10
#define NOTA_11_MIDI 11
#define NOTA_12_MIDI 12
#define NOTA_13_MIDI 13
#define NOTA_14_MIDI 14
#define NOTA_15_MIDI 15

// Estos valores dependen de como están cableadas las entradas analógicas en el controlador
// FIJOS - NO TOCAR!!!
#define CC_0_HW  0
#define CC_1_HW  1
#define CC_2_HW  2
#define CC_3_HW  3
#define CC_4_HW  4
#define CC_5_HW  5
#define CC_6_HW  6
#define CC_7_HW  7
#define CC_8_HW  8
#define CC_9_HW  9
#define CC_10_HW 10
#define CC_11_HW 11
#define CC_12_HW 12
#define CC_13_HW 13
#define CC_14_HW 14
#define CC_15_HW 15

// Estos son los valores de Control Change que se quieren recibir en la aplicación DAW.
// AJUSTABLES SEGÚN APLICACIÓN
#define CC_0_MIDI  0
#define CC_1_MIDI  1
#define CC_2_MIDI  2
#define CC_3_MIDI  3
#define CC_4_MIDI  4
#define CC_5_MIDI  5
#define CC_6_MIDI  6
#define CC_7_MIDI  7
#define CC_8_MIDI  8
#define CC_9_MIDI  9
#define CC_10_MIDI 10
#define CC_11_MIDI 11
#define CC_12_MIDI 12
#define CC_13_MIDI 13
#define CC_14_MIDI 14
#define CC_15_MIDI 15

// Estos son los identificadores de cada LED en hardware.
// FIJOS - NO TOCAR!!!
#define N_LED_HW_0 0
#define N_LED_HW_1 1
#define N_LED_HW_2 2
#define N_LED_HW_3 3
#define N_LED_HW_4 4
#define N_LED_HW_5 5
#define N_LED_HW_6 6
#define N_LED_HW_7 7
#define N_LED_HW_8 8
#define N_LED_HW_9 9
#define N_LED_HW_10 10
#define N_LED_HW_11 11
#define N_LED_HW_12 12
#define N_LED_HW_13 13
#define N_LED_HW_14 14
#define N_LED_HW_15 15

// Estos son los valores de notas a los que responde cada LED. Deben estar apareados los LEDs que sean indicadores de botones.
// AJUSTABLES SEGÚN APLICACIÓN
#define N_LED_MIDI_0 0
#define N_LED_MIDI_1 1
#define N_LED_MIDI_2 2
#define N_LED_MIDI_3 3
#define N_LED_MIDI_4 4
#define N_LED_MIDI_5 5
#define N_LED_MIDI_6 6
#define N_LED_MIDI_7 7
#define N_LED_MIDI_8 8
#define N_LED_MIDI_9 9
#define N_LED_MIDI_10 10
#define N_LED_MIDI_11 11
#define N_LED_MIDI_12 12
#define N_LED_MIDI_13 13
#define N_LED_MIDI_14 14
#define N_LED_MIDI_15 15

// FIJOS - NO TOCAR!!!
#define NOTE_ON   127
#define NOTE_OFF  0

// AJUSTABLES SEGÚN APLICACIÓN
#define CANAL_MIDI_NOTES  1                              // DEFINIR CANAL MIDI A UTILIZAR
#define CANAL_MIDI_CC     1                              // DEFINIR CANAL MIDI A UTILIZAR
#define CANAL_MIDI_LEDS   1                              // DEFINIR CANAL MIDI A UTILIZAR

// SOLO CAMBIAR SI SE EXPERIMENTAN OSCILACIONES NO DESEADAS DE LOS CC QUE CORRESPONDEN A LOS POTENCIOMETROS O FADERS 
#define UMBRAL_RUIDO        1                      // Ventana de ruido para las entradas analógicas. Si entrada < entrada+umbral o 
                                                   //                                                   entrada > entrada-umbral descarta la lectura.
// AJUSTABLE SEGÚN APLICACIÓN
#define VELOCITY_LIM_TITILA 64                     // Limite de velocity para definir parpadeo - 0            - APAGADO
                                                   //                                            1 a LIMITE   - TITILA
                                                   //                                            LIMITE a 127 - ENCENDIDO
// PARA DEBUG O USO CON HAIRLESS MIDI
#define VELOCIDAD_SERIAL 115200                    // Velocidad de transferencia (bits/seg) de la comunicación serial


// A PARTIR DE ACA YA NO HAY PARÁMETROS DE CONFIGURACIÓN
unsigned int anteriorMillis = 0;                   // Variable para guardar ms
boolean tiempo_on = 0;                             // Tiempo que llevan encendidos los leds, para el parpadeo

MuxShield muxShield;                               // Objeto de la clase MuxShield, para leer los multiplexores

// Variables para los registros de desplazamiento
const int dataPin = 8;                             // Pin de datos (DS) conectado al pin 14 del 74HC595
const int latchPin = 9;                            // Pin de latch (ST_CP) conectado al pin 12 del 74HC595
const int clockPin = 10;                           // Pin de clock (SH_CP) conectado al pin 11 del 74HC595
boolean registros[NUM_LEDS];                       // Estado de los LEDs (HIGH - 1 o LOW - 0)
boolean leds_que_titilan[NUM_LEDS];                // Si está en 1, el LED titila. Si está en 0, no titila

// Datos analógicos
byte velocity[NUM_MUX][NUM_CANALES_MUX];               // Variable de 1 byte que guarda la velocity actual de cada canal
byte velocityPrev[NUM_MUX][NUM_CANALES_MUX];           // Variable de 1 byte que guarda la velocity anterior de cada canal

// Contadores, flags
int mux, canal;              // Contadores y flags

void setup() {
  // Setear pines de salida
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
 
  pinMode(ENTRADA_DIGITAL, INPUT);                // Estas dos líneas setean el pin analógico que recibe las entradas digitales como pin digital y
  digitalWrite(ENTRADA_DIGITAL, HIGH);            // setea el resistor de Pull-Up en el mismo
  
  // Guardar cantidad de ms desde el encendido
  anteriorMillis = millis();

  for (mux = 0; mux < NUM_MUX; mux++) {                // Setear todos los velocity a 0
    for (canal = 0; canal < NUM_CANALES_MUX; canal++) {
      velocity[mux][canal] = 0;
      velocityPrev[mux][canal] = 0;
    }
  }

  clearRegisters();                             // Se limpia el array registros (todos a LOW)
  writeRegisters();                             // Se envían los datos al 595
  // COMENTAR LA SIGUIENTE LINEA PARA DEBUGGEAR CON SERIAL ///////////////////////////////////////////////////////////////////////
  //MIDI.begin(MIDI_CHANNEL_OMNI); MIDI.turnThruOff();  // Se inicializa la comunicación MIDI.
                                                      // Por default, la librería de Arduino MIDI tiene el THRU en ON, y NO QUEREMOS ESO!
                                                      
  // DESCOMENTAR LA SIGUIENTE LINEA PARA DEBUGGEAR CON SERIAL ///////////////////////////////////////////////////////////////////////                                                      
  Serial.begin(VELOCIDAD_SERIAL);                  // Se inicializa la comunicación serial. Descomentar para usar con Hairless MID
}

void loop() {  
  // COMENTAR LA SIGUIENTE LINEA PARA DEBUGGEAR CON SERIAL ///////////////////////////////////////////////////////////////////////
  // leer_MIDI();
  // DESCOMENTAR LA SIGUIENTE LINEA PARA DEBUGGEAR CON SERIAL ///////////////////////////////////////////////////////////////////////
  encender_leds_serial();
  titilarLeds();
  leerEntradas();
}

// Lee el canal midi, note y velocity, y actualiza el estado de los leds.
void leer_MIDI(void) {
  if (MIDI.read(CANAL_MIDI_LEDS)) {         // ¿Llegó un mensaje MIDI?
    byte led_number = MIDI.getData1();  // Capturo la nota
    byte led_vel = MIDI.getData2();
    switch (MIDI.getType())                  // Identifica que tipo de mensaje llegó (NoteOff, NoteOn, AfterTouchPoly, ControlChange, ProgramChange, AfterTouchChannel, PitchBend, SystemExclusive)
    {
      case midi::NoteOn:                       // En caso que sea NoteOn,
        if (led_vel > VELOCITY_LIM_TITILA) {     // Si la velocity es un valor entre LIMITE y 127, el led no parpadea
          switch(led_number){
            case N_LED_MIDI_0:
              setLed_595(N_LED_HW_0, HIGH); leds_que_titilan[N_LED_HW_0] = 0; break;      // Encender el LED correspondiente y no dejar que titile
            case N_LED_MIDI_1:
              setLed_595(N_LED_HW_1, HIGH); leds_que_titilan[N_LED_HW_1] = 0; break;            
            case N_LED_MIDI_2:
              setLed_595(N_LED_HW_2, HIGH); leds_que_titilan[N_LED_HW_2] = 0; break;            
            case N_LED_MIDI_3:
              setLed_595(N_LED_HW_3, HIGH); leds_que_titilan[N_LED_HW_3] = 0; break;            
            case N_LED_MIDI_4:
              setLed_595(N_LED_HW_4, HIGH); leds_que_titilan[N_LED_HW_4] = 0; break;            
            case N_LED_MIDI_5:
              setLed_595(N_LED_HW_5, HIGH); leds_que_titilan[N_LED_HW_5] = 0; break;            
            case N_LED_MIDI_6:
              setLed_595(N_LED_HW_6, HIGH); leds_que_titilan[N_LED_HW_6] = 0; break;            
            case N_LED_MIDI_7:
              setLed_595(N_LED_HW_7, HIGH); leds_que_titilan[N_LED_HW_7] = 0; break;            
            case N_LED_MIDI_8:
              setLed_595(N_LED_HW_8, HIGH); leds_que_titilan[N_LED_HW_8] = 0; break;            
            case N_LED_MIDI_9:
              setLed_595(N_LED_HW_9, HIGH); leds_que_titilan[N_LED_HW_9] = 0; break;            
            case N_LED_MIDI_10:
              setLed_595(N_LED_HW_10, HIGH); leds_que_titilan[N_LED_HW_10] = 0; break;           
            case N_LED_MIDI_11:
              setLed_595(N_LED_HW_11, HIGH); leds_que_titilan[N_LED_HW_11] = 0; break;           
            case N_LED_MIDI_12:
              setLed_595(N_LED_HW_12, HIGH); leds_que_titilan[N_LED_HW_12] = 0; break;           
            case N_LED_MIDI_13:
              setLed_595(N_LED_HW_13, HIGH); leds_que_titilan[N_LED_HW_13] = 0; break;           
            case N_LED_MIDI_14:
              setLed_595(N_LED_HW_14, HIGH); leds_que_titilan[N_LED_HW_14] = 0; break;           
            case N_LED_MIDI_15:
              setLed_595(N_LED_HW_15, HIGH); leds_que_titilan[N_LED_HW_15] = 0; break;            // Encender el LED correspondiente              
            default: break;  
          }
        }
        else if (led_vel > 0 && led_vel <= VELOCITY_LIM_TITILA)     // Si la velocity es mayor a cero, y menor a LIMITE,
          switch(led_number){
            case N_LED_MIDI_0:
              leds_que_titilan[N_LED_HW_0] = 1; break;      // Encender el LED correspondiente y hacer que titile
            case N_LED_MIDI_1:
              leds_que_titilan[N_LED_HW_1] = 1; break;            
            case N_LED_MIDI_2:
              leds_que_titilan[N_LED_HW_2] = 1; break;            
            case N_LED_MIDI_3:
              leds_que_titilan[N_LED_HW_3] = 1; break;            
            case N_LED_MIDI_4:
              leds_que_titilan[N_LED_HW_4] = 1; break;            
            case N_LED_MIDI_5:
              leds_que_titilan[N_LED_HW_5] = 1; break;            
            case N_LED_MIDI_6:
              leds_que_titilan[N_LED_HW_6] = 1; break;            
            case N_LED_MIDI_7:
              leds_que_titilan[N_LED_HW_7] = 1; break;            
            case N_LED_MIDI_8:
              leds_que_titilan[N_LED_HW_8] = 1; break;            
            case N_LED_MIDI_9:
              leds_que_titilan[N_LED_HW_9] = 1; break;            
            case N_LED_MIDI_10:
              leds_que_titilan[N_LED_HW_10] = 1; break;           
            case N_LED_MIDI_11:
              leds_que_titilan[N_LED_HW_11] = 1; break;           
            case N_LED_MIDI_12:
              leds_que_titilan[N_LED_HW_12] = 1; break;           
            case N_LED_MIDI_13:
              leds_que_titilan[N_LED_HW_13] = 1; break;           
            case N_LED_MIDI_14:
              leds_que_titilan[N_LED_HW_14] = 1; break;           
            case N_LED_MIDI_15:
              leds_que_titilan[N_LED_HW_15] = 1; break;            // Encender el LED correspondiente              
            default: break;  
          }          
        else {                                                      // Si las anteriores no se cumplen, entonces es velocity = 0, es decir NoteOff,
          switch(led_number){
            case N_LED_MIDI_0:
              setLed_595(N_LED_HW_0, LOW); leds_que_titilan[N_LED_HW_0] = 0; break;      // Encender el LED correspondiente y no dejar que titile
            case N_LED_MIDI_1:
              setLed_595(N_LED_HW_1, LOW); leds_que_titilan[N_LED_HW_1] = 0; break;            
            case N_LED_MIDI_2:
              setLed_595(N_LED_HW_2, LOW); leds_que_titilan[N_LED_HW_2] = 0; break;            
            case N_LED_MIDI_3:
              setLed_595(N_LED_HW_3, LOW); leds_que_titilan[N_LED_HW_3] = 0; break;            
            case N_LED_MIDI_4:
              setLed_595(N_LED_HW_4, LOW); leds_que_titilan[N_LED_HW_4] = 0; break;            
            case N_LED_MIDI_5:
              setLed_595(N_LED_HW_5, LOW); leds_que_titilan[N_LED_HW_5] = 0; break;            
            case N_LED_MIDI_6:
              setLed_595(N_LED_HW_6, LOW); leds_que_titilan[N_LED_HW_6] = 0; break;            
            case N_LED_MIDI_7:
              setLed_595(N_LED_HW_7, LOW); leds_que_titilan[N_LED_HW_7] = 0; break;            
            case N_LED_MIDI_8:
              setLed_595(N_LED_HW_8, LOW); leds_que_titilan[N_LED_HW_8] = 0; break;            
            case N_LED_MIDI_9:
              setLed_595(N_LED_HW_9, LOW); leds_que_titilan[N_LED_HW_9] = 0; break;            
            case N_LED_MIDI_10:
              setLed_595(N_LED_HW_10, LOW); leds_que_titilan[N_LED_HW_10] = 0; break;           
            case N_LED_MIDI_11:
              setLed_595(N_LED_HW_11, LOW); leds_que_titilan[N_LED_HW_11] = 0; break;           
            case N_LED_MIDI_12:
              setLed_595(N_LED_HW_12, LOW); leds_que_titilan[N_LED_HW_12] = 0; break;           
            case N_LED_MIDI_13:
              setLed_595(N_LED_HW_13, LOW); leds_que_titilan[N_LED_HW_13] = 0; break;           
            case N_LED_MIDI_14:
              setLed_595(N_LED_HW_14, LOW); leds_que_titilan[N_LED_HW_14] = 0; break;           
            case N_LED_MIDI_15:
              setLed_595(N_LED_HW_15, LOW); leds_que_titilan[N_LED_HW_15] = 0; break;            // Encender el LED correspondiente              
            default: break;  
          }
        }
        break;
      default:
        break;
    }
  }
}

void encender_leds_serial(void){
// EL DEBUG FUNCIONA ENVIANDO DESDE EL TERMINAL SERIE UN NUMERO DEL 0-9 O UNA LETRA DE a-f O DE A-F (10-15) QUE CORRESPONDE AL LED QUE QUEREMOS QUE CAMBIE DE ESTADO
  // CADA VEZ QUE LLEGA EL NUMERO, SE CAMBIA DE UN ESTADO AL SIGUIENTE
  // APAGADO -> TITILANDO -> ENCENDIDO -> APAGADO
    static unsigned int leds[NUM_LEDS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    if (Serial.available()){                // ¿Llegó un mensaje Serial?
  
      byte led_number = Serial.read();  // Capturo la nota
      switch(led_number){
        case 'a': led_number = 10; break; case 'A': led_number = 10; break;
        case 'b': led_number = 11; break; case 'B': led_number = 11; break;
        case 'c': led_number = 12; break; case 'C': led_number = 12; break;
        case 'd': led_number = 13; break; case 'D': led_number = 13; break;
        case 'e': led_number = 14; break; case 'E': led_number = 14; break;
        case 'f': led_number = 15; break; case 'F': led_number = 15; break;
        case 'o': 
          for (int i = 0; i<NUM_LEDS; i++){
            setLed_595(i, HIGH);
          }
          delay(1000);
          for (int i = 0; i<NUM_LEDS; i++){
            setLed_595(i, LOW);
          }
          return;
        default: led_number %= 16; break;
      }
      leds[led_number]++;
      leds[led_number] %= 3;
      if (leds[led_number] == 0){
        setLed_595(led_number, LOW);                                // Apagar el LED correspondiente
        leds_que_titilan[led_number] = 0;                         // Actualizar el flag para que no titile más
        Serial.print("LED "); Serial.print(led_number); Serial.println(" apagado.");
      }
      else if (leds[led_number] == 1){
        leds_que_titilan[led_number] = 1;                         // Actualizar el flag para que titile
        Serial.print("LED "); Serial.print(led_number); Serial.println(" titilando.");
      }
      else if (leds[led_number] == 2){
        setLed_595(led_number, HIGH);                               // Encender el LED correspondiente
        leds_que_titilan[led_number] = 0;                         // Actualizar el flag para que no titile más
        Serial.print("LED "); Serial.print(led_number); Serial.println(" encendido.");
      }
  
    }
}

// Esta función actualiza el estado de los LEDs que titilan
void titilarLeds(void) {
  unsigned int i = 0;
  unsigned int actualMillis = millis();              // temporizador que carga el número de milisegundos (ms) transcurridos desde el encendido
  if (actualMillis - anteriorMillis > 300) {         // Si transcurrieron más de 300 ms desde la ultima actualización,
    for (i = 0; i < NUM_LEDS; i++) {                   // Recorrer todos los LEDs
      anteriorMillis = actualMillis;                     // Actualizo el contador de ms usado en la comparación
      if (leds_que_titilan[i]) {                        // Si corresponde titilar este LED,
        if (tiempo_on) {                                   // Y estaba encendido,
          setLed_595(i, HIGH);                               // Se apaga
        }
        else {                                           // Si estaba apagado,
          setLed_595(i, LOW);                              // Se enciende
        }
      }
    }
    tiempo_on = !tiempo_on;
  }
}

// Esta función lee todas las entradas y almacena los valores de cada una en la matriz 'velocity'.
// Compara con los valores previos, almacenados en 'velocityPrev', y si cambian, envía una NoteOn o NoteOff (entradas digitales) o un ControlChange (entradas analógicas)
void leerEntradas(void) {
  static bool noteOn[NUM_CANALES_MUX] = {};
  for (mux = 0; mux < NUM_MUX; mux++) {                                           // Recorro el numero de multiplexores, definido al principio
    for (canal = 0; canal < NUM_CANALES_MUX; canal++) {                                // Recorro todos los canales de cada multiplexor
      // ENTRADAS ANALÓGICAS
      if (mux == MUX_ANALOGICO) {                                                 // Si es un potenciómetro/fader/sensor,
        // CÓDIGO PARA LECTURA DE ENTRADAS ANALÓGICAS /////////////////////////////////////////////////////////////////////
        unsigned int analogData = muxShield.analogReadMS(mux + 1, canal);            // Leer entradas analógicas 'muxShield.analogReadMS(N_MUX,N_CANAL)'
        velocity[mux][canal] = analogData >> 3;                                      // El valor leido va de 0-1023. Convertimos a 0-127, dividiendo por 8.
        
        if (esRuido(canal) == 0) {                                         // Si lo que leo no es ruido
          // COMENTAR LA SIGUIENTE LINEA PARA DEBUGGEAR CON SERIAL ///////////////////////////////////////////////////////////////////////
          // enviarControlChangeMidi(canal);
          // DESCOMENTAR LA SIGUIENTE LINEA PARA DEBUGGEAR CON SERIAL ///////////////////////////////////////////////////////////////////////          
          enviarControlChangeSerial(canal);
          // FIN COMENTARIO ///////////////////////////////////////////////////////////////////////////////////////////
        }
        else continue;
      }
      // FIN CÓDIGO PARA LECTURA DE ENTRADAS ANALÓGICAS /////////////////////////////////////////////////////////////////
      // ENTRADAS DIGITALES
      if (mux == MUX_DIGITAL) {                                                    // Si es un pulsador,
        // CÓDIGO PARA LECTURA DE ENTRADAS DIGITALES /////////////////////////////////////////////////////////////////////
        velocity[mux][canal] = muxShield.digitalReadMS(mux + 1, canal);               // Leer entradas analógicas 'muxShield.analogReadMS(N_MUX,N_CANAL)'
        
        if (velocity[mux][canal] != velocityPrev[mux][canal]) {                        // Me interesa la lectura, si cambió el estado del botón,
          if (!velocity[mux][canal] && !noteOn[canal]) {                               // Si leo un voltaje mayor a 2.5V (botón apagado - PULLUP) (0 -> 0V, 127 -> 5V), y
            noteOn[canal] = 1;
            // Se envía NOTE ON
            // COMENTAR LA SIGUIENTE LINEA PARA DEBUGGEAR CON SERIAL ///////////////////////////////////////////////////////////////////////
            //enviarNoteMidi(canal, NOTE_ON);
            // DESCOMENTAR LA SIGUIENTE LINEA PARA DEBUGGEAR CON SERIAL ///////////////////////////////////////////////////////////////////////            
            enviarNoteSerial(canal, NOTE_ON);
          }
          else if (noteOn[canal]) {
            noteOn[canal] = 0;
            // COMENTAR LA SIGUIENTE LINEA PARA DEBUGGEAR CON SERIAL ///////////////////////////////////////////////////////////////////////
            //enviarNoteMidi(canal, NOTE_OFF);
            // DESCOMENTAR LA SIGUIENTE LINEA PARA DEBUGGEAR CON SERIAL ///////////////////////////////////////////////////////////////////////
            enviarNoteSerial(canal, NOTE_OFF);
          }
        }
        // FIN CÓDIGO PARA LECTURA DE ENTRADAS DIGITALES /////////////////////////////////////////////////////////////////////
      }
      velocityPrev[mux][canal] = velocity[mux][canal];             // Almacenar velocidad actual como anterior
    }
  }
}

void enviarNoteMidi(unsigned int nota, unsigned int veloc) {
  switch (nota) {
    case NOTA_0_HW:  MIDI.sendNoteOn(NOTA_0_MIDI, veloc, CANAL_MIDI_NOTES); break;
    case NOTA_1_HW:  MIDI.sendNoteOn(NOTA_1_MIDI, veloc, CANAL_MIDI_NOTES); break;
    case NOTA_2_HW:  MIDI.sendNoteOn(NOTA_2_MIDI, veloc, CANAL_MIDI_NOTES); break;
    case NOTA_3_HW:  MIDI.sendNoteOn(NOTA_3_MIDI, veloc, CANAL_MIDI_NOTES);  break;
    case NOTA_4_HW:  MIDI.sendNoteOn(NOTA_4_MIDI, veloc, CANAL_MIDI_NOTES);  break;
    case NOTA_5_HW:  MIDI.sendNoteOn(NOTA_5_MIDI, veloc, CANAL_MIDI_NOTES);  break;
    case NOTA_6_HW:  MIDI.sendNoteOn(NOTA_6_MIDI, veloc, CANAL_MIDI_NOTES);  break;                        // MAPEO DE ORDEN DE BOTONES, SEGÚN COMO QUEDARON LOS BOTONES EN HARDWARE Y COMO LOS QUEREMOS EN SOFTWARE
    case NOTA_7_HW:  MIDI.sendNoteOn(NOTA_7_MIDI, veloc, CANAL_MIDI_NOTES);  break;                        // case NRO_BOTON_HARDWARE: MIDI.sendNoteOn(NRO_BOTON_SOFTWARE, velocity, canal);
    case NOTA_8_HW:  MIDI.sendNoteOn(NOTA_8_MIDI, veloc, CANAL_MIDI_NOTES);  break;
    case NOTA_9_HW:  MIDI.sendNoteOn(NOTA_9_MIDI, veloc, CANAL_MIDI_NOTES);  break;
    case NOTA_10_HW:  MIDI.sendNoteOn(NOTA_10_MIDI, veloc, CANAL_MIDI_NOTES);  break;
    case NOTA_11_HW:  MIDI.sendNoteOn(NOTA_11_MIDI, veloc, CANAL_MIDI_NOTES);  break;
    case NOTA_12_HW:  MIDI.sendNoteOn(NOTA_12_MIDI, veloc, CANAL_MIDI_NOTES);  break;
    case NOTA_13_HW:  MIDI.sendNoteOn(NOTA_13_MIDI, veloc, CANAL_MIDI_NOTES);  break;
    case NOTA_14_HW:  MIDI.sendNoteOn(NOTA_14_MIDI, veloc, CANAL_MIDI_NOTES);  break;
    case NOTA_15_HW:  MIDI.sendNoteOn(NOTA_15_MIDI, veloc, CANAL_MIDI_NOTES);  break;
  }
  return;
}
void enviarNoteSerial(unsigned int nota, unsigned int veloc) {
  veloc /= 127;
  switch (nota) {
    case NOTA_0_HW:  Serial.print("BOTON NOTE ON"); Serial.print("  Nota: "); Serial.print(NOTA_0_MIDI); Serial.print("  Velocity: "); Serial.println(veloc); break;
    case NOTA_1_HW:  Serial.print("BOTON NOTE ON"); Serial.print("  Nota: "); Serial.print(NOTA_1_MIDI); Serial.print("  Velocity: "); Serial.println(veloc); break;
    case NOTA_2_HW:  Serial.print("BOTON NOTE ON"); Serial.print("  Nota: "); Serial.print(NOTA_2_MIDI); Serial.print("  Velocity: "); Serial.println(veloc); break;
    case NOTA_3_HW:  Serial.print("BOTON NOTE ON"); Serial.print("  Nota: "); Serial.print(NOTA_3_MIDI); Serial.print("  Velocity: "); Serial.println(veloc); break;
    case NOTA_4_HW:  Serial.print("BOTON NOTE ON"); Serial.print("  Nota: "); Serial.print(NOTA_4_MIDI); Serial.print("  Velocity: "); Serial.println(veloc); break;
    case NOTA_5_HW:  Serial.print("BOTON NOTE ON"); Serial.print("  Nota: "); Serial.print(NOTA_5_MIDI); Serial.print("  Velocity: "); Serial.println(veloc); break;
    case NOTA_6_HW:  Serial.print("BOTON NOTE ON"); Serial.print("  Nota: "); Serial.print(NOTA_6_MIDI); Serial.print("  Velocity: "); Serial.println(veloc); break;
    case NOTA_7_HW:  Serial.print("BOTON NOTE ON"); Serial.print("  Nota: "); Serial.print(NOTA_7_MIDI); Serial.print("  Velocity: "); Serial.println(veloc); break;
    case NOTA_8_HW:  Serial.print("BOTON NOTE ON"); Serial.print("  Nota: "); Serial.print(NOTA_8_MIDI); Serial.print("  Velocity: "); Serial.println(veloc); break;
    case NOTA_9_HW:  Serial.print("BOTON NOTE ON"); Serial.print("  Nota: "); Serial.print(NOTA_9_MIDI); Serial.print("  Velocity: "); Serial.println(veloc); break;
    case NOTA_10_HW:  Serial.print("BOTON NOTE ON"); Serial.print("  Nota: "); Serial.print(NOTA_10_MIDI); Serial.print("  Velocity: "); Serial.println(veloc); break;
    case NOTA_11_HW:  Serial.print("BOTON NOTE ON"); Serial.print("  Nota: "); Serial.print(NOTA_11_MIDI); Serial.print("  Velocity: "); Serial.println(veloc); break;
    case NOTA_12_HW:  Serial.print("BOTON NOTE ON"); Serial.print("  Nota: "); Serial.print(NOTA_12_MIDI); Serial.print("  Velocity: "); Serial.println(veloc); break;
    case NOTA_13_HW:  Serial.print("BOTON NOTE ON"); Serial.print("  Nota: "); Serial.print(NOTA_13_MIDI); Serial.print("  Velocity: "); Serial.println(veloc); break;
    case NOTA_14_HW:  Serial.print("BOTON NOTE ON"); Serial.print("  Nota: "); Serial.print(NOTA_14_MIDI); Serial.print("  Velocity: "); Serial.println(veloc); break;
    case NOTA_15_HW:  Serial.print("BOTON NOTE ON"); Serial.print("  Nota: "); Serial.print(NOTA_15_MIDI); Serial.print("  Velocity: "); Serial.println(veloc); break;
  }
}  
// Remapea las entradas analógicas y las envía por MIDI
void enviarControlChangeMidi(unsigned int nota) {
  switch (nota) {
    case CC_0_HW:  MIDI.sendControlChange(CC_0_MIDI, velocity[mux][canal], CANAL_MIDI_CC); break;
    case CC_1_HW:  MIDI.sendControlChange(CC_1_MIDI, velocity[mux][canal], CANAL_MIDI_CC); break;
    case CC_2_HW:  MIDI.sendControlChange(CC_2_MIDI, velocity[mux][canal], CANAL_MIDI_CC); break;
    case CC_3_HW:  MIDI.sendControlChange(CC_3_MIDI, velocity[mux][canal], CANAL_MIDI_CC);  break;
    case CC_4_HW:  MIDI.sendControlChange(CC_4_MIDI, velocity[mux][canal], CANAL_MIDI_CC);  break;
    case CC_5_HW:  MIDI.sendControlChange(CC_5_MIDI, velocity[mux][canal], CANAL_MIDI_CC);  break;
    case CC_6_HW:  MIDI.sendControlChange(CC_6_MIDI, velocity[mux][canal], CANAL_MIDI_CC);  break; // MAPEO DE ORDEN DE ENTRADAS ANALOGICAS, SEGÚN COMO QUEDARON EN HARDWARE
    case CC_7_HW:  MIDI.sendControlChange(CC_7_MIDI, velocity[mux][canal], CANAL_MIDI_CC);  break; // Y COMO LOS QUEREMOS EN SOFTWARE
    case CC_8_HW:  MIDI.sendControlChange(CC_8_MIDI, velocity[mux][canal], CANAL_MIDI_CC);  break; // case NRO_BOTON_HARDWARE: MIDI.sendControlChange(NRO_ANALOG_MIDI, velocity, canal);
    case CC_9_HW:  MIDI.sendControlChange(CC_9_MIDI, velocity[mux][canal], CANAL_MIDI_CC);  break;
    case CC_10_HW:  MIDI.sendControlChange(CC_10_MIDI, velocity[mux][canal], CANAL_MIDI_CC);  break;
    case CC_11_HW:  MIDI.sendControlChange(CC_11_MIDI, velocity[mux][canal], CANAL_MIDI_CC);  break;
    case CC_12_HW:  MIDI.sendControlChange(CC_12_MIDI, velocity[mux][canal], CANAL_MIDI_CC);  break;
    case CC_13_HW:  MIDI.sendControlChange(CC_13_MIDI, velocity[mux][canal], CANAL_MIDI_CC);  break;
    case CC_14_HW:  MIDI.sendControlChange(CC_14_MIDI, velocity[mux][canal], CANAL_MIDI_CC);  break;
    case CC_15_HW:  MIDI.sendControlChange(CC_15_MIDI, velocity[mux][canal], CANAL_MIDI_CC);  break;
    
  }
  return;
}

void enviarControlChangeSerial(unsigned int nota) {
  switch (nota) {
    case CC_0_HW:  Serial.print("Numero de pote: "); Serial.print(CC_0_MIDI); Serial.print("  Valor: "); Serial.println(velocity[mux][canal]); break;
    case CC_1_HW:  Serial.print("Numero de pote: "); Serial.print(CC_1_MIDI); Serial.print("  Valor: "); Serial.println(velocity[mux][canal]); break;
    case CC_2_HW:  Serial.print("Numero de pote: "); Serial.print(CC_2_MIDI); Serial.print("  Valor: "); Serial.println(velocity[mux][canal]); break;
    case CC_3_HW:  Serial.print("Numero de pote: "); Serial.print(CC_3_MIDI); Serial.print("  Valor: "); Serial.println(velocity[mux][canal]); break;
    case CC_4_HW:  Serial.print("Numero de pote: "); Serial.print(CC_4_MIDI); Serial.print("  Valor: "); Serial.println(velocity[mux][canal]); break;
    case CC_5_HW:  Serial.print("Numero de pote: "); Serial.print(CC_5_MIDI); Serial.print("  Valor: "); Serial.println(velocity[mux][canal]); break;
    case CC_6_HW:  Serial.print("Numero de pote: "); Serial.print(CC_6_MIDI); Serial.print("  Valor: "); Serial.println(velocity[mux][canal]); break;
    case CC_7_HW:  Serial.print("Numero de pote: "); Serial.print(CC_7_MIDI); Serial.print("  Valor: "); Serial.println(velocity[mux][canal]); break;
    case CC_8_HW:  Serial.print("Numero de pote: "); Serial.print(CC_8_MIDI); Serial.print("  Valor: "); Serial.println(velocity[mux][canal]); break;
    case CC_9_HW:  Serial.print("Numero de pote: "); Serial.print(CC_9_MIDI); Serial.print("  Valor: "); Serial.println(velocity[mux][canal]); break;
    case CC_10_HW:  Serial.print("Numero de pote: "); Serial.print(CC_10_MIDI); Serial.print("  Valor: "); Serial.println(velocity[mux][canal]); break;
    case CC_11_HW:  Serial.print("Numero de pote: "); Serial.print(CC_11_MIDI); Serial.print("  Valor: "); Serial.println(velocity[mux][canal]); break;
    case CC_12_HW:  Serial.print("Numero de pote: "); Serial.print(CC_12_MIDI); Serial.print("  Valor: "); Serial.println(velocity[mux][canal]); break;
    case CC_13_HW:  Serial.print("Numero de pote: "); Serial.print(CC_13_MIDI); Serial.print("  Valor: "); Serial.println(velocity[mux][canal]); break;
    case CC_14_HW:  Serial.print("Numero de pote: "); Serial.print(CC_14_MIDI); Serial.print("  Valor: "); Serial.println(velocity[mux][canal]); break;
    case CC_15_HW:  Serial.print("Numero de pote: "); Serial.print(CC_15_MIDI); Serial.print("  Valor: "); Serial.println(velocity[mux][canal]); break;
  }
  return;
  
}

// Funcion para filtrar el ruido analógico de los pontenciómetros. Guarda 3 valores para detectar si el valor crece o decrece intencionalmente, o si
// es ruido de inestabilidad que baila entre dos valores +-1 (canal==0)?UMBRAL_RUIDO:UMBRAL_RUIDO+1
unsigned int esRuido(unsigned int nota) {
  static unsigned int prev2[NUM_CANALES_MUX] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  if ((velocity[mux][nota] > velocityPrev[mux][nota] + UMBRAL_RUIDO) || (velocity[mux][nota] < velocityPrev[mux][nota] - UMBRAL_RUIDO)) {
    return 0;
  }
  else if ((velocity[mux][nota] == velocityPrev[mux][nota] + UMBRAL_RUIDO) || (velocity[mux][nota] == velocityPrev[mux][nota] - UMBRAL_RUIDO)) {
    if (prev2[nota] == velocity[mux][nota]) {
      return 1;
    }
    else {
      prev2[nota] = velocityPrev[mux][nota];
      return 0;
    }
  }
  else return 1;
}

// Prender o apagar un solo led
void setLed_595(int num_led, int estado) {
  registros[num_led] = estado;
  writeRegisters();
}

// Apagar todos los LEDs
void clearRegisters() {
  for (int i = NUM_LEDS - 1; i >=  0; i--) {
    registros[i] = LOW;
  }
}

// Actualizar el registro de desplazamiento. Actualiza el estado de los LEDs
// Antes de llamar a ésta función, el array 'registros' debería contener el estado requerido para TODOS los LEDs
void writeRegisters() {
  digitalWrite(latchPin, LOW);                  // Se baja la línea de latch para avisar al 595 que se van a enviar datos

  for (int i = NUM_LEDS - 1; i >=  0; i--) {    // Se recorren todos los LEDs,
    digitalWrite(clockPin, LOW);                  // Se baja "manualmente" la línea de clock

    int val = registros[i];                       // Se recupera el estado del LED

    digitalWrite(dataPin, val);                   // Se escribe el estado del LED en la línea de datos
    digitalWrite(clockPin, HIGH);                 // Se levanta la línea de clock para el próximo bit
  }
  digitalWrite(latchPin, HIGH);                 // Se vuelve a poner en HIGH la línea de latch para avisar que no se enviarán mas datos
}


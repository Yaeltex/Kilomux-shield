
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
*  - Jorge Crowe
*  - Lucas Leal
*   - Dimitri Diakopoulos
*/

#ifndef KILO_MUX_H
#define KILO_MUX_H

// For Arduino 1.0 and earlier
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "KiloMux.h"
/*
 * Inclusión de librerías. 
 */
#include "MuxShield.h"
#include "NewPing.h"
#include "MIDI.h"
#include "midi_Defs.h"
#include "midi_Namespace.h"
#include "midi_Settings.h"
//#include <midi_Message.h>      // Si no compila porque le falta midi_message.h, descomentar esta linea

/*
 * A contiuación está la sección de configuración de funcionalidades del código para tu aplicación.
 * Cada etiqueta #define representa una funcionalidad que se añade o se modifica.
 * La línea puede estar comentada con doble barra al inicio (//). Si una etiqueta está comentada, todas las líneas del código que estén encerradas por un
 *    #ifdef
 *    ...
 *    ...
 *    ...
 *    #endif
 * no estarán presentes en el código que se cargue en la Arduino. El compilador se encarga de esto ;)
 * Lo bueno de usar las etiquetas #define de este modo, es que no se carga código extra en la Arduino, al no usar 
 * alguna funcionalidad.
 */
 
void setup(); // Esto es para solucionar el bug que tiene Arduino al usar los #ifdef del preprocesador
 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Descomentar la próxima línea si el compilador no encuentra MIDI
// MIDI_CREATE_DEFAULT_INSTANCE()
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dejar descomentada sólo una de las tres lineas siguientes para definir el tipo de comunicación
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#define COMUNICACION_MIDI          // Para enviar mensajes a través de HIDUINO o por hardware
//#define HAIRLESS_MIDI            // Para enviar mensajes midi por USB hacia Hairless MIDI
#define COMUNICACION_SERIAL      // Para debuggear con el Monitor Serial
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Comentar la siguiente linea si no se usa sensor de ultrasonido HC_SR04
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define CON_ULTRASONIDO
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#define TOGGLE            // Dejar descomentada si se quiere que los botones actúen como TOGGLE
                            // Con el modo TOGGLE, los botones envían un sólo mensaje (ON o OFF) cada vez que se los suelta
                            // Con el modo normal, cada botón envía un mensaje al ser presionar (ON) y otro al soltarlo (OFF)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#define BOTONES_CC        // Descomentar si se quiere que los botones envíen mensajes CC
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define SHIFTERS            // Dejar descomentada si se usan los shifters
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(SHIFTERS)
//#define TOGGLE_SHIFTERS     // Dejar descomentada si se usan los shifters con modo TOGGLE 
#endif  // endif SHIFTERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * En la siguiente sección se encuentran las etiquetas que representan algún valor. 
 * El compilador reemplaza todas aquellas que encuentre en el código, por su respectivo valor.
 * La ventaja de usar etiquetas #define en vez de variables, es que las etiquetas no ocupan memoria RAM del microcontrolador.
 * Se especifica cuáles son modificables y cuáles NO SE DEBEN MODIFICAR :)
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NO MODIFICAR
#define MUX_A                 0
#define MUX_B                 1
#define MUX_A_ENTRADA         A0 // Pin analógico por dónde se lee el MUX A
#define MUX_B_ENTRADA         A1 // Pin analógico por dónde se lee el MUX B

// ¿A qué multiplexor del shield están conectados las entradas analógicas (potes, faders)?
#define MUX_ANALOGICO         MUX_B 
#define ENTRADA_ANALOGICA     MUX_B_ENTRADA 
// Si no tenés botones, descomenta las siguientes dos líneas y comenta los defines MUX_DIGITAL y ENTRADA_DIGITAL
//#define MUX_ANALOGICO_2       MUX_B 
//#define ENTRADA_ANALOGICA_2   MUX_B_ENTRADA 

// ¿A qué multiplexor del shield están conectados las entradas digitales (botones)?
#define MUX_DIGITAL           MUX_A   
#define ENTRADA_DIGITAL       MUX_A_ENTRADA 

#define NUM_MUX               2            // Número de multiplexores a direccionar - Si usas el KiloMux Shield, poner "2"
#define NUM_CANALES_MUX       16           // Número de canales en cada mux         - Si usas el KiloMux Shield, poner "16"

#define NUM_595               2            // Número de integrados 74HC595          - Si usas el KiloMux Shield, poner "2"
#define NUM_LEDS_X_BANCO      NUM_595*8    // Número de LEDs en total, NO CAMBIAR
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Estos son los valores de notas que se quieren envíar cuando se detecte un cambio en cada canal del multiplexor
 * La primera columna indica el número de nota (o CC) MIDI que se enviará, al presionar el botón conectado en el canal indicado.
 */
 // AJUSTABLES SEGÚN APLICACIÓN
 byte mapeoNotes[NUM_CANALES_MUX] = {0,        // CANAL 0
                                     1,        // CANAL 1
                                     2,        // CANAL 2
                                     3,        // CANAL 3
                                     4,        // CANAL 4
                                     5,        // CANAL 5
                                     6,        // CANAL 6
                                     7,        // CANAL 7
                                     8,        // CANAL 8
                                     9,        // CANAL 9
                                     10,       // CANAL 10
                                     11,       // CANAL 11
                                     12,       // CANAL 12
                                     13,       // CANAL 13
                                     14,       // CANAL 14
                                     15};      // CANAL 15

// Estos son los valores de Control Change que se quieren recibir en la aplicación DAW.
// AJUSTABLES SEGÚN APLICACIÓN
#if !defined(MUX_ANALOGICO_2)
byte mapeoCC[NUM_CANALES_MUX] = {0,        // CANAL 0
                                 1,        // CANAL 1
                                 2,        // CANAL 2
                                 3,        // CANAL 3
                                 4,        // CANAL 4
                                 5,        // CANAL 5
                                 6,        // CANAL 6
                                 7,        // CANAL 7
                                 8,        // CANAL 8
                                 9,        // CANAL 9
                                 10,       // CANAL 10
                                 11,       // CANAL 11
                                 12,       // CANAL 12
                                 13,       // CANAL 13
                                 14,       // CANAL 14
                                 15};      // CANAL 15 
#else
// SÓLO TOCAR ESTE ARRAY SI EL CONTROLADOR NO TIENE BOTONES, Y TIENE MÁS DE 16 SEÑALES ANALÓGICAS
byte mapeoCC[NUM_CANALES_MUX+8] = {0,        // CANAL 0
                                   1,        // CANAL 1
                                   2,        // CANAL 2
                                   3,        // CANAL 3
                                   4,        // CANAL 4
                                   5,        // CANAL 5
                                   6,        // CANAL 6
                                   7,        // CANAL 7
                                   8,        // CANAL 8
                                   9,        // CANAL 9
                                   10,       // CANAL 10
                                   11,       // CANAL 11
                                   12,       // CANAL 12
                                   13,       // CANAL 13
                                   14,       // CANAL 14
                                   15,       // CANAL 15
                                   16,       // CANAL 16
                                   17,       // CANAL 17
                                   18,       // CANAL 18 
                                   19,       // CANAL 19
                                   20,       // CANAL 20
                                   21,       // CANAL 21
                                   22,       // CANAL 22
                                   23};      // CANAL 23 
#endif

/*
 ***************************************************************************************************************************************************
 * Procedimiento para mapear LEDs presionando botones. Comenzar con el led 0, y sucesivamente ir subiendo hasta el 15. 
 * Para mapear los LEDs se deben haber ordenado primero los botones asociados a ellos.
 * NO CARGAR EL CODIGO HASTA HABER ORDENADO TODOS LOS LEDS.
 * 
 * 0- Asegurarse que el programa cargado está configurado para enviar datos por Serial, y no MIDI. 
 *    Si recibimos caracteres sin sentido en el monitor serial, es posible que en realidad el programa esté enviando MIDI. 
 *    En ese caso, cargar el programa nuevamente, con el #define COMUNICACION_SERIAL descomentado y COMUNICACION_MIDI comentado.
 * 1- Abrir el monitor serial.
 * 2- Presionar un botón.
 * 3- Fijarse qué LED enciende, y presionar el botón asociado a ese LED. Al presionar ese botón, el monitor serial nos dirá qué número es. 
 *    Si no hay botón asociado, en este punto debemos decidir la nota que queremos que encienda el LED.
 * 4- Ir a la fila correspondiente al numero del segundo botón en la siguiente lista ("mapeoLeds").
 * 5- Reemplazar el número de la columna de la izquierda por el del primer botón en el paso 1.
 * 6- Repetir los pasos 2 a 5 para todos los LEDs. Si al presionar un botón, no se enciende un LED, seguir con el siguiente.
 * 7- Una vez ordenados TODOS los LEDs, cargar el código y verificar que los LEDs están ordenados presionando cada botón.
 * 
 * EJEMPLO:
 * 1- Abrir el monitor serial.
 * 2- Presionar el botón 0.
 * 3- Se encendió un LED. Presionar el botón asociado a ese LED y el monitor serial dice que es el botón 12.
 * 4- Ir a la fila LED 12.
 * 5- En la columna de la izquierda reemplazar el valor original por el número 0, que es el que se envió al principio.
 * 6- Continuar con el botón 1.
 * 
 * Alternativamente, se pueden mapear los LEDs a través del monitor serial. Esto es necesario cuando tenemos mas LEDs que botones.
 * El procedimiento es el siguiente, a partir del paso 2: 
 * 2- Se envía desde el monitor serial un número del 00 al 15 (enviar con ENTER) y se identifica el LED encendido. 
 *    NOTA: Si no se enciende ningún LED continuar con el siguiente número.
 * 3- Se presiona el botón asociado a ese LED. El monitor serial nos dirá que nota envía ese botón, que será la que debe encender ese LED.
 *    Si no hay botón asociado, en este punto debemos decidir la nota que queremos que encienda el LED.
 * 4- Ir a la fila correspondiente a esa nota.
 * 5- Reemplazar el número de la columna de la izquierda por el que enviamos al principio a través del monitor serial.
 * 6- Repetir los pasos 2 a 5 para todos los LEDs.
 * 7- Una vez ordenados TODOS los LEDs, cargar el código y verificar que los LEDs están ordenados presionando cada botón o enviando del 00 al 15 
 *    desde el monitor serial
 *    
 * EJEMPLO:
 * 2- En el campo de la parte superior del monitor serial escribir el número 00. Enviar con ENTER
 * 3- Se encendió un LED. Presionar el botón asociado a ese LED y el monitor serial dice que es el botón 12.
 * 4- Ir a la fila LED 12.
 * 5- En la columna de la izquierda reemplazar el valor original por el número 0, que es el que se envió al principio.
 * 6- Continuar con el número 01.   
 ***************************************************************************************************************************************************
 */
byte mapeoLeds[NUM_LEDS_X_BANCO] = {0,        // LED 0 - NOTE 0
                                    1,        // LED 1 - NOTE 1 
                                    2,        // LED 2 - NOTE 2
                                    3,        // LED 3 - NOTE 3
                                    4,        // LED 4 - NOTE 4
                                    5,        // LED 5 - NOTE 5
                                    6,        // LED 6 - NOTE 6
                                    7,        // LED 7 - NOTE 7
                                    8,        // LED 8 - NOTE 8
                                    9,        // LED 9 - NOTE 9
                                    10,       // LED 10 - NOTE 10
                                    11,       // LED 11 - NOTE 11
                                    12,       // LED 12 - NOTE 12
                                    13,       // LED 13 - NOTE 13
                                    13,       // LED 14 - NOTE 14
                                    15};      // LED 15 - NOTE 15

// AJUSTABLES SEGUN APLICACION
// Identificador de HW de los botones que actúan como shifters de CC.
#if defined(SHIFTERS)             // Si están definidos los shifters  
  #define BOTON_BANCO_1  0      
  #define BOTON_BANCO_2  1        // Comentar si no se usa 
  #define BOTON_BANCO_3  2        // Comentar si no se usa 
  #define BOTON_BANCO_4  3        // Comentar si no se usa 
  
  #define LED_SHIFTER_1  0
  #define LED_SHIFTER_2  1      
  #define LED_SHIFTER_3  2      
  #define LED_SHIFTER_4  3  

  #define SHIFTER_1      0
  #define SHIFTER_2      1      
  #define SHIFTER_3      2      
  #define SHIFTER_4      3 

  #define BANCO_0        0
  #define BANCO_1        1
  #define BANCO_2        2
  #define BANCO_3        3
  #define BANCO_4        4
      
  // NO MODIFICAR
  #if defined(BOTON_BANCO_4)
    #define BANCOS 5
  #elif defined(BOTON_BANCO_3)
    #define BANCOS 4
  #elif defined(BOTON_BANCO_2)
    #define BANCOS 3
  #elif defined(BOTON_BANCO_1)
    #define BANCOS 2
  #endif   // endif BOTONES_BANCO_4
#else    // else SHIFTERS
    #define BANCOS 1
#endif   // endif SHIFTERS

#define SALTO_SHIFTER 16    // Este define indica el salto que se realiza al presionar un shifter

// NO CAMBIAR
#define NOTE_ON   127
#define NOTE_OFF  0

#define ON  1
#define OFF 0

// MODIFICAR SEGÚN APLICACIÓN
#define CANAL_MIDI_NOTES  1                              // DEFINIR CANAL MIDI A UTILIZAR
#define CANAL_MIDI_CC     1                              // DEFINIR CANAL MIDI A UTILIZAR
#define CANAL_MIDI_LEDS   1                              // DEFINIR CANAL MIDI A UTILIZAR

// NO CAMBIAR
// Para la función de filtrado de ruido analógico
#define ANALOGO_CRECIENDO   1
#define ANALOGO_DECRECIENDO 0

// AJUSTABLE - Si hay ruido que varía entre valores (+- 1, +- 2, +- 3...) colocar el umbral en (1, 2, 3...)
#define UMBRAL_RUIDO        1                      // Ventana de ruido para las entradas analógicas. Si entrada < entrada+umbral o 
                                                   //                                                   entrada > entrada-umbral descarta la lectura.

// FIJO - NO CAMBIAR!!!
#define LED_APAGADO   0
#define LED_TITILANDO 1
#define LED_ENCENDIDO 2

// AJUSTABLE                                                   
#define INTERVALO_LEDS      300                    // Intervalo de intermitencia
#define VELOCITY_LIM_TITILA 64                     // Limite de velocity en el MIDI INpara definir parpadeo - 0            - APAGADO
                                                   //                                                         1 a LIMITE   - TITILA
                                                   //                                                         LIMITE a 127 - ENCENDIDO

// DEFINES PARA SENSOR ULTRASONICO                                                                                                    
#if defined(CON_ULTRASONIDO)                      
  // AJUSTABLES
  #define CC_ULTRASONIDO       100
  #define MAX_DISTANCIA        45        // Maxima distancia que se desea medir (en centimetros). El sensor mide hasta 400-500cm.
  #define MIN_DISTANCIA        2         // Minima distancia que se desea medir (en centimetros). El sensor mide desde 1cm.
  #define DELAY_ULTRAS         15        // Delay entre dos pings del sensor (en milisegundos). Mantener arriba de 10.
  #define UMBRAL_DIFERENCIA_US 80        // 
  #define FILTRO_US            3         // Cantidad de valores almacenados para el filtro. Cuanto más grande, mejor el suavizado y  más lenta la lectura.
  // FIJOS - NO CAMBIAR!!!
  #define TRIGGER_PIN          13        // Pin de arduino conectado al pin de trigger del sensor
  #define ECHO_PIN             12        // Pin de arduino conectado al pin de echo del sensor
  #define PIN_BOTON_ACT_US     10        // Pin de arduino conectado al botón que activa el sensor
  #define PIN_LED_ACT_US       11        // Pin de arduino conectado al led que indica la activación del sensor
  #define MAX_US               MAX_DISTANCIA*US_ROUNDTRIP_CM   // Maximo tiempo en microsegundos que dura el pulso en retornar
  #define MIN_US               MIN_DISTANCIA*US_ROUNDTRIP_CM   // Minimo tiempo en microsegundos que dura el pulso en retornar
#endif    // endif CON_ULTRASONIDO

// AJUSTABLE
#define VELOCIDAD_SERIAL 115200                    // Velocidad de transferencia (bits/seg) de la comunicación serial. Para HAIRLESS_MIDI usar 115200. Recordar sincronizar con el receptor!

//////////////////////////////////////////////////////////////////////////////////////////
// VARIABLES y OBJETOS ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
#if defined(CON_ULTRASONIDO)
NewPing sensorUS(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCIA); // Instancia de NewPing para el sensor US.
int ccSensorValuePrev[FILTRO_US];                       
#endif

MuxShield muxShield;                               // Objeto de la clase MuxShield, para leer los multiplexores

// Variables para los registros de desplazamiento
const int dataPin = 6;                             // Pin de datos (DS) conectado al pin 14 del 74HC595
const int latchPin = 7;                            // Pin de latch (ST_CP) conectado al pin 12 del 74HC595
const int clockPin = 8;                            // Pin de clock (SH_CP) conectado al pin 11 del 74HC595
boolean registros[NUM_LEDS_X_BANCO];                       // Estado de los LEDs (HIGH - 1 o LOW - 0) para uso de las funciones que escriben el 74HC595.
unsigned int estadoLeds[BANCOS][NUM_LEDS_X_BANCO];    // Estado de todos los bancos de LEDs

// Prescalers para el ADC
const unsigned char PS_16 = (1 << ADPS2);
const unsigned char PS_32 = (1 << ADPS2) | (1 << ADPS0);
const unsigned char PS_64 = (1 << ADPS2) | (1 << ADPS1);
const unsigned char PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
  
// Datos analógicos
byte lecturas[NUM_MUX][NUM_CANALES_MUX];               // Variable de 1 byte que guarda la lectura actual de cada entrada
byte lecturasPrev[NUM_MUX][NUM_CANALES_MUX];           // Variable de 1 byte que guarda la lectura anterior de cada entrada

// Contadores, flags
byte mux, canal, i, j;                        // Contadores para recorrer los multiplexores
bool tiempo_on = 0;                          // Tiempo que llevan encendidos los leds, para el parpadeo
bool cambioBanco = 0, cambioEstadoLeds = 0;  // Flags para actualización de LEDs
unsigned long anteriorMillis = 0;            // Variable para guardar ms
bool estadoBoton[BANCOS][NUM_CANALES_MUX];           // Estado de los botones
bool estadoShifter[BANCOS];                  // Estado de los botones que son shifters de banco
unsigned int bancoActivo = 0;                // Variable que almacena el banco actual.
unsigned int bancoAnt = 0;                   // Variable que registra guarda el último banco

///////////////////////////////////////////////////////////////////////////////////////////
// Prototipos de funciones ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

// Manejo de LEDs /////////////////////////////////////////////////////////////////////////
void EscribirRegistros595(void);
void LimpiarRegistros595(void);
void SetearLed595(uint8_t num_led, uint8_t estado);
void ActualizarLeds(void);
void TitilarLeds(void);
///////////////////////////////////////////////////////////////////////////////////////////

// Recibir comandos para prender LEDs /////////////////////////////////////////////////////
void EncenderLedsSerial(void);
void LeerMidi(void);
///////////////////////////////////////////////////////////////////////////////////////////

// Velocidad de sampleo ADC ///////////////////////////////////////////////////////////////
void SetADCprescaler(byte setting);
///////////////////////////////////////////////////////////////////////////////////////////

// Enviar valores MIDI ////////////////////////////////////////////////////////////////////
void EnviarNoteMidi(unsigned int nota, unsigned int veloc);
void EnviarControlChangeMidi(unsigned int nota);
///////////////////////////////////////////////////////////////////////////////////////////

// Enviar valores al monitor SERIAL ///////////////////////////////////////////////////////
void EnviarNoteSerial(unsigned int nota, unsigned int veloc);
void EnviarControlChangeSerial(unsigned int nota);
///////////////////////////////////////////////////////////////////////////////////////////

// Lectura de entradas (Sensor US, Multiplexores) /////////////////////////////////////////
void LeerUltrasonico(void);
void LeerEntradas(void);
unsigned int EsRuido(unsigned int nota);
///////////////////////////////////////////////////////////////////////////////////////////

// Páginas ////////////////////////////////////////////////////////////////////////////////
void CambiarBanco(int shifter);
byte EsShifter(unsigned int nBoton);
///////////////////////////////////////////////////////////////////////////////////////////

#endif //KILO_MUX_H
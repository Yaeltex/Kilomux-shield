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
#include <MuxShield.h>
#include <NewPing.h>
#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>
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
//#define SHIFTERS            // Dejar descomentada si se usan los shifters
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
                                    14,       // LED 14 - NOTE 14
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
#define VELOCIDAD_SERIAL 115200            // Velocidad de transferencia (bits/seg) de la comunicación serial. Para HAIRLESS_MIDI usar 115200. Recordar sincronizar con el receptor!

//////////////////////////////////////////////////////////////////////////////////////////
// VARIABLES y OBJETOS ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

// Instancias de objetos //////////////////////////////////////////////////////////////////
#if defined(CON_ULTRASONIDO)
NewPing sensorUS(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCIA); // Instancia de NewPing para el sensor US.
int ccSensorValuePrev[FILTRO_US];                       // Array para el filtrado de la lectura del sensor            
#endif

MuxShield muxShield;                       // Objeto de la clase MuxShield, para leer los multiplexores
///////////////////////////////////////////////////////////////////////////////////////////

// Variables para los registros de desplazamiento /////////////////////////////////////////
const int dataPin = 6;                     // Pin de datos (DS) conectado al pin 14 del 74HC595
const int latchPin = 7;                    // Pin de latch (ST_CP) conectado al pin 12 del 74HC595
const int clockPin = 8;                    // Pin de clock (SH_CP) conectado al pin 11 del 74HC595
boolean registros[NUM_LEDS_X_BANCO];       // Estado de los LEDs (HIGH - 1 o LOW - 0) para uso de las funciones que escriben el 74HC595.
byte estadoLeds[BANCOS][NUM_LEDS_X_BANCO]; // Estado de todos los bancos de LEDs
///////////////////////////////////////////////////////////////////////////////////////////

// Prescalers para el ADC /////////////////////////////////////////////////////////////////
const unsigned char PS_16 = (1 << ADPS2);
const unsigned char PS_32 = (1 << ADPS2) | (1 << ADPS0);
const unsigned char PS_64 = (1 << ADPS2) | (1 << ADPS1);
const unsigned char PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
///////////////////////////////////////////////////////////////////////////////////////////

// Datos analógicos
byte lecturas[NUM_MUX][NUM_CANALES_MUX];     // Variable de 1 byte que guarda la lectura actual de cada entrada
byte lecturasPrev[NUM_MUX][NUM_CANALES_MUX]; // Variable de 1 byte que guarda la lectura anterior de cada entrada
///////////////////////////////////////////////////////////////////////////////////////////

// Contadores, flags //////////////////////////////////////////////////////////////////////
byte mux, canal, i, j;                       // Contadores para recorrer los multiplexores
bool flagTitilar = 0;                        // Tiempo que llevan encendidos los leds, para el parpadeo
bool cambioBanco = 0, cambioEstadoLeds = 0;  // Flags para actualización de LEDs
unsigned long anteriorMillis = 0;            // Variable para guardar ms
bool estadoBoton[BANCOS][NUM_CANALES_MUX];   // Estado de los botones
bool estadoShifter[BANCOS];                  // Estado de los botones que son shifters de banco
unsigned int bancoActivo = 0;                // Variable que almacena el banco actual.
unsigned int bancoAnt = 0;                   // Variable que registra guarda el último banco
///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////
// Prototipos de funciones ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

// Manejo de LEDs /////////////////////////////////////////////////////////////////////////
void EscribirRegistros595(void);                    // Enviar los datos por serial a los 595
void LimpiarRegistros595(void);                     // Setear todos los registros a LOW
void SetearLed595(uint8_t num_led, uint8_t estado); // Cambiar el estado de una salida de los 595
void ActualizarLeds(void);                          // Encender o apagar un led si algún flag lo indica
void TitilarLeds(void);                             // Actualizar el estado de los LEDs que titilan
///////////////////////////////////////////////////////////////////////////////////////////

// Recibir comandos para prender LEDs /////////////////////////////////////////////////////
void EncenderLedsSerial(void);                      // Interpretar la llegada de datos por Serial y actualizar el estado de los LEDs
void LeerMidi(void);                                // Interpretar la llegada de datos por MIDI y actualizar el estado de los LEDs  
///////////////////////////////////////////////////////////////////////////////////////////

// Velocidad de sampleo ADC ///////////////////////////////////////////////////////////////
void SetADCprescaler(byte setting);                 // Setear el divisor para la frecuencia de muestreo del ADC
///////////////////////////////////////////////////////////////////////////////////////////

// Enviar valores MIDI ////////////////////////////////////////////////////////////////////
void EnviarNoteMidi(unsigned int nota, unsigned int veloc);   // Enviar un mensaje Note On/Off con velocity "veloc"
void EnviarControlChangeMidi(unsigned int nota);              // Enviar un mensaje Control Change
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

///////////////////////////////////////////////////////////////////////////////////////////
/// SETUP (INICIALIZACIÓN) ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void setup() {
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
  ActualizarLeds();
   
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
  if(cambioBanco || cambioEstadoLeds){  
    cambioBanco = 0; cambioEstadoLeds = 0;  
    
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
}

// Esta función actualiza el estado de los LEDs que titilan
void TitilarLeds(void) {
  unsigned int led = 0;
  for (led = 0; led < NUM_LEDS_X_BANCO; led++) {                   // Recorrer todos los LEDs
    if (estadoLeds[bancoActivo][led] == LED_TITILANDO) {     // Si corresponde titilar este LED,
      if (flagTitilar) {                                   // Y estaba encendido,
        SetLed595(led, HIGH);                               // Se apaga
      }
      else {                                           // Si estaba apagado,
        SetLed595(led, LOW);                              // Se enciende
      }
    }
  }
  anteriorMillis = millis();                     // Actualizo el contador de ms usado en la comparación
  flagTitilar = !flagTitilar;
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
      char inputBuffer[3] = {' ',' ', 0};
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



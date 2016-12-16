/*
 * Autor: Franco Grassano - YAELTEX
 * Fecha: 18/02/2016
 * ---
 * INFORMACIÓN DE LICENCIA
 * Kilomux Shield por Yaeltex se distribuye bajo una licencia
 * Creative Commons Atribución-CompartirIgual 4.0 Internacional - http://creativecommons.org/licenses/by-sa/4.0/
 * ----
 * Description: Uso de un sensor ultrasónico HC-SR04. Se lee el sensor y se envía un mensaje MIDI CC, cuando la lectura cambia.
 *              Se realiza un filtrado de media móvil para suavizar las lecturas, y se previene que el sensor lea 0 cuando el 
 *              objeto se aleja demasiado, o sale de la línea de sensado.
 *              Usa un botón para activar o desactivar el sensado, y un LED para indicar ese estado.
 *              Este ejemplo está pensado para el uso con el Kilomux Shield.
 * 
 * Librería para el manejo del sensor de ultrasonido tomada de http://playground.arduino.cc/Code/NewPing
 */
 
 /*
 * Inclusión de librerías. 
 */
#include <NewPing.h>
#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>
//#include <midi_Message.h>      // Si no compila porque le falta midi_message.h, descomentar esta linea

/*
 * A continuación está la sección de configuración de funcionalidades del código para tu aplicación.
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
#define SERIAL_COMMS        // Para debuggear con el Monitor Serial
//#define HAIRLESS_MIDI     // Para debuggear usando HARILESS MIDI, mandando MIDI por Serial, NO RECOMENDADO
//#define MIDI_USB          // Modo de producción, para enviar MIDI por USB
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * En la siguiente sección se encuentran las etiquetas que representan algún valor. 
 * El compilador reemplaza todas aquellas que encuentre en el código, por su respectivo valor.
 * La ventaja de usar etiquetas #define en vez de variables, es que las etiquetas no ocupan memoria RAM del microcontrolador.
 * Se especifica cuáles son modificables y cuáles NO SE DEBEN MODIFICAR :)
 */

// DEFINES PARA SENSOR ULTRASONICO                                                                                                    
  // AJUSTABLES
  #define CC_ULTRASONIDO       100
  #define MAX_DISTANCIA        100        // Maxima distancia que se desea medir (en centimetros). El sensor mide hasta 400-500cm.
  #define MIN_DISTANCIA        2         // Minima distancia que se desea medir (en centimetros). El sensor mide desde 1cm.
  #define DELAY_ULTRAS         15        // Delay entre dos pings del sensor (en milisegundos). Mantener arriba de 10.
  #define UMBRAL_DIFERENCIA_US 80        // 
  #define FILTRO_US            3         // Cantidad de valores almacenados para el filtro. Cuanto más grande, mejor el suavizado y  más lenta la lectura.
  #define TRIGGER_PIN          13        // Pin de arduino conectado al pin de trigger del sensor
  #define ECHO_PIN             12        // Pin de arduino conectado al pin de echo del sensor
  #define PIN_BOTON_ACT_US     10        // Pin de arduino conectado al botón que activa el sensor
  #define PIN_LED_ACT_US       11        // Pin de arduino conectado al led que indica la activación del sensor
  // NO CAMBIAR!!!
  #define MAX_US               MAX_DISTANCIA*US_ROUNDTRIP_CM   // Maximo tiempo en microsegundos que dura el pulso en retornar
  #define MIN_US               MIN_DISTANCIA*US_ROUNDTRIP_CM   // Minimo tiempo en microsegundos que dura el pulso en retornar

// AJUSTABLE
#define VELOCIDAD_SERIAL 115200            // Velocidad de transferencia (bits/seg) de la comunicación serial. Para HAIRLESS_MIDI usar 115200. Recordar sincronizar con el receptor!

//////////////////////////////////////////////////////////////////////////////////////////
// VARIABLES y OBJETOS ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

// Instancias de objetos //////////////////////////////////////////////////////////////////
NewPing sensorUS(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCIA); // Instancia de NewPing para el sensor US.
int ccSensorValuePrev[FILTRO_US];                       // Array para el filtrado de la lectura del sensor            

///////////////////////////////////////////////////////////////////////////////////////////
// Prototipos de funciones ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

// Enviar valores al monitor SERIAL ///////////////////////////////////////////////////////
void EnviarControlChangeSerial(unsigned int nota);
///////////////////////////////////////////////////////////////////////////////////////////

// Lectura de entradas (Sensor US, Multiplexores) /////////////////////////////////////////
int LeerUltrasonico(void);

///////////////////////////////////////////////////////////////////////////////////////////
/// SETUP (INICIALIZACIÓN) ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  pinMode(PIN_LED_ACT_US, OUTPUT);         // LED indicador para el sensor de distancia
  pinMode(PIN_BOTON_ACT_US,INPUT_PULLUP);  // Botón de activación para el sensor de distancia

#if defined(MIDI_USB)
  MIDI.begin(MIDI_CHANNEL_OMNI); MIDI.turnThruOff();  // Se inicializa la comunicación MIDI.
                                                      // Por default, la librería de Arduino MIDI tiene el THRU en ON, y NO QUEREMOS ESO!
#elif defined(SERIAL_COMMS)
  Serial.begin(VELOCIDAD_SERIAL);                  // Se inicializa la comunicación serial. 
#elif defined(HAIRLESS_MIDI)
  MIDI.begin(MIDI_CHANNEL_OMNI); MIDI.turnThruOff();  // Se inicializa la comunicación MIDI.
                                                      // Por default, la librería de Arduino MIDI tiene el THRU en ON, y NO QUEREMOS ESO!
#endif
  Serial.begin(VELOCIDAD_SERIAL);                  // Se inicializa la comunicación serial. 
}

//////////////////////////////////////////////////////////////////////////////////////////
// PROGRAMA PRINCIPAL ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  // Función que lee los datos que reporta el sensor ultrasónico, filtra y envía la información MIDI si hay cambios.
  int lecturaSensor = LeerUltrasonico();
  if (lecturaSensor > 0){
    #if defined(MIDI_USB)|defined(HAIRLESS_MIDI)
      MIDI.sendControlChange(CC_ULTRASONICO, lecturaSensor, 1);                                     // Enviar CC por MIDI CC en canal 1
    #elif defined(SERIAL_COMMS)
      Serial.print("Valor arrojado por el sensor ultrasonico: "); Serial.println(lecturaSensor);    // Envío SERIAL
    #endif
  }  
}

int LeerUltrasonico(void){
  static unsigned int antMillisUltraSonico = 0;    // Variable usada para almacenar los milisegundos desde el último Ping al sensor
  static bool estadoBotonUSAnt = 1;                // Inicializo inactivo (entrada activa baja)
  static bool estadoBotonUS = 1;                   // Inicializo inactivo (entrada activa baja)
  static bool estadoLed = 0;                       // Inicializo inactivo (salida activa alta)
  static bool sensorActivado = 1;                  // Inicializo inactivo (variable interna)
  static unsigned int indiceFiltro = 0;            // Indice que recorre los valores del filtro de suavizado
    
  int microSeg = 0;                                // Contador de microsegundos para el ping del sensor.
  
//  // Este codigo verifica si se presionó el botón y activa o desactiva el sensor cada vez que se presiona
//  estadoBotonUS = digitalRead(PIN_BOTON_ACT_US);
//  if(estadoBotonUS == LOW && estadoBotonUSAnt == HIGH){    // Si el botón previamente estaba en estado alto, y ahora esta en estado bajo, quiere decir que paso de estar no presionado a presionado (activo bajo)
//    estadoBotonUSAnt = LOW;                                // Actualizo el estado previo
//    sensorActivado = !sensorActivado;                      // Activo o desactivo el sensor
//    estadoLed = !estadoLed;                                // Cambio el estado del LED
//    digitalWrite(PIN_LED_ACT_US, estadoLed);               // Y actualizo la salida
//  }
//  else if(estadoBotonUS == HIGH && estadoBotonUSAnt == LOW){  // Si el botón previamente estaba en estado bajo, y ahora esta en estado alto, quiere decir que paso de estar presionado a no presionado 
//    estadoBotonUSAnt = HIGH;                                  // Actualizo el estado previo
//  } 
  
  if(sensorActivado){                                       // Si el sensor está activado
    if (millis()-antMillisUltraSonico > DELAY_ULTRAS){      // y transcurrió el delay minimo entre lecturas
      antMillisUltraSonico = millis();                      
      microSeg = sensorUS.ping();                           // Sensar el tiempo que tarda el pulso de ultrasonido en volver. Se recibe el valor el us.
      int ccSensorValue = map(constrain(microSeg, MIN_US, MAX_US), MIN_US, MAX_US, 0, 130);   // Esta línea convierte el valor en microsegundos que arroja la función ping(), a un valor de 0 a 127, para enviar como mensaje MIDI
      
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
        if(ccSensorValue != ccSensorValuePrev[indiceFiltro])      
          return ccSensorValue;
      }
      else return -1;        
    }
  }
}

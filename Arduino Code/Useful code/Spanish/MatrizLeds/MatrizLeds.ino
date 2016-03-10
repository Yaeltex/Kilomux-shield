/*
 * Autor: Franco Grassano - YAELTEX
 * Fecha: 18/02/2016
 * ---
 * INFORMACIÓN DE LICENCIA
 * Kilomux Shield por Yaeltex se distribuye bajo una licencia
 * Creative Commons Atribución-CompartirIgual 4.0 Internacional - http://creativecommons.org/licenses/by-sa/4.0/
 * ----
 * Description: Este código maneja una matriz de LEDs con las salidas del Kilomux shield.
 *              El ejemplo 
 *              Es importante conectar las columnas y las filas a un puerto de salida diferente (OUT-1 y OUT-2), y especificar 
 *              a cual de ellos se conectaron las columnas en la etiqueta COL_PORT.
 * 
 * Librería para el manejo del sensor de ultrasonido tomada de http://playground.arduino.cc/Code/NewPing
 */
 
#include <Kilomux.h>
#include <KilomuxDefs.h>

#define ROW_INTERVAL 2500                 // Intervalo de multiplexado de las filas en microsegundos. Con 2.5 milisegundos, ningún humando percibe la multiplexación de las filas. Yo no la percibo. Vos tampoco deberías. A menos que tengas supervista.
#define ROWS      4                       // Número de filas de la matriz. Modificar según sea necesario (Máximo 8)
#define COLS      4                       // Número de columnas de la matriz. Modificar según sea necesario (Máximo 8)

#define COL_PORT  2                       // Puerto del kilomux shield al que se conectan las columnas.

#define N_LEDS    ROWS*COLS               // Número total de LEDs

#define LED_OFF   0
#define LED_ON    1

#define BLINK_INTERVAL  300                // Velocidad de recorrido de la matriz

Kilomux KmShield;                          // Instancia de la clase Kilomux

unsigned int ledRows[] = {1, 2, 3, 4};     // Salidas del Kilomux donde conectamos las columnas. Agregá o quita las que correspondan, dentro del array.
unsigned int ledCols[] = {9, 10, 11, 12};  // Salidas del Kilomux donde conectamos las columnas. Agregá o quita las que correspondan, dentro del array.

uint8_t ledMatrixState[ROWS][COLS];        // Esta matriz es siempre una copia de la matriz de LEDs real, con 1's donde hay un LED encendido y 0's donde hay uno apagado.
uint8_t led = 0;                           // Contador
unsigned long lastMillis = 0;              // Instante en milisegundos

void setup() {
  // Inicialización de Hardware
  for (int i=0; i<ROWS; i++){
    KmShield.digitalWriteKm(ledRows[i], LOW);       // Filas a LOW, apaga la fila
  }
  KmShield.digitalWritePortKm(B11111111, COL_PORT); // Columnas a HIGH, bloquea los LEDs

  // Inicialización de software
  for(int i = 0; i<N_LEDS; i++){
    setMatrixLed(i,LED_OFF);            // Apagar los LEDs, por software
  }

  setMatrixLed(0, LED_ON);              // Prender el LED 0, la primera vez. 
  
  // Get current time
  lastMillis = millis();      // Guardar el instante actual
}

void loop() {
  if(millis() - lastMillis > BLINK_INTERVAL){     // Si es momento de cambiar de LED
    lastMillis = millis();                          // Guardar el instante actual
    setMatrixLed(led, LOW);                         // Apagar el LED anterior
    led++;                                          // Avanzar contador
    led %= N_LEDS;                                  // Buffer circular, si sobrepasa el número de LEDs, vuelve a 0.
    setMatrixLed(led, HIGH);                        // Prender el siguiente LED
  }
  
  // Actualizar la matriz //////////////////////////////////////////////
  updateLedMatrix();
}

// Esta función recibe el número de LED a encender o a apagar, y actualiza el estado de la matriz
void setMatrixLed(uint8_t nLed, uint8_t state){   
  ledMatrixState[nLed/COLS][nLed%COLS] = state;     // Ésta línea transforma el número de LED a actualizar, en una coordenada (fila, columna)
  return;                                           // Por ejemplo, el número 5, si se tienen 4 filas y 4 columnas -> fila = 5/4 = 1 y columna = 5%4 = 1 -> entonces el LED 5, será el de la fila 1, columna 1.
}                                                   //              el número 13, si se tienen 2 filas y 8 columnas -> fila = 13/8 = 1 y columna = 13%8 = 5 -> entonces el LED 13, será el de la fila 1, columna 5.

/*
 * Esta función se encarga de multiplexar las filas, encendiendo una a la vez, alternando rápidamente entre ellas.
 * La persistencia de la visión (POV) hace que veamos todas encendidas en simultáneo.
 * 
 */
void updateLedMatrix(void){
  static unsigned long prevMicros = 0;                // Instante previo para multiplexado
  static unsigned int row;                            // Fila actual
  byte colState = 0;                                  // Byte con el estado de cada columna, en cada bit

  unsigned long currentMicros = micros();             // Obtener instante actual
  if(currentMicros-prevMicros > ROW_INTERVAL){        // Si es tiempo de pasar de fila
    prevMicros = currentMicros;                         // Guardar instante actual
    KmShield.digitalWriteKm(ledRows[row++], LOW);       // Apagar la fila actual y pasar a la siguiente
    row %= ROWS;                                        // Si se alcanzó la última fila, volver a la 0.
    colState &= 0x00;                                   // Inicializar las columnas a 0 -> B00000000
    for(int col = 0; col<COLS; col++){                  // Para cada columna,
      int thisLed = ledMatrixState[row][col];             // Obtener el estado del LED de esa columna
      colState |= thisLed<<(7-col);                       // Hacer un "o" lógico entre el estado del LED, y el estado de todas las columnas. 
    }                                                     // Por ejemplo, si la columna 5 (empezando desde 0) debe encenderse, esta fila deja colState en B00000100 (1<<7-5 = 1<<2)
    KmShield.digitalWritePortKm(~colState, COL_PORT); // Las columnas deben ponerse a 0V para encender los LEDs, por lo que se actualiza el puerto dónde se conectaron las columnas, con el byte negado (B11111011)
    KmShield.digitalWriteKm(ledRows[row], HIGH);      // Encender la nueva fila
  }  
  return;
}

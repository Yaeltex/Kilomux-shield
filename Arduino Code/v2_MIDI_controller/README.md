En esta carpeta se halla el código necesario (librerías y sketch) para usar con la versión 2 del KiloMux Shield (la que parece de verdad).

El código <b><i>v2midiController</i></b> incorpora grandes mejoras con respecto a la versión anterior, entre ellas la posibilidad de usar hasta 5 páginas o bancos mediante el uso y la especificación de algunos botones como shifters. Esto permite, por ejemplo, si se usan 16 potenciómetros, controlar hasta 80 parámetros de un DAW. El número de shifters es a su vez expandible.

Otra mejora en el código es el mapeo dinámico de los componentes, mediante tablas, en lugar de los defines que usaba la versión 1. Esto simplifico y redujo mucho el código para lograr la correspondencia entre el identificador del canal del multiplexor, y el identificador MIDI que se requiere.

Por el momento, este código no hace uso de la librería "KiloMux Shield".
<b>UPDATE: ¡La versión 3 de este sketch está subida!</b>

Este código usa las siguientes librerías
- [MuxShield](https://github.com/Yaeltex/Kilomux-shield/tree/master/Arduino%20Code/v2_MIDI_controller/Librer%C3%ADas/MuxShield) de [Mayhew Labs] (http://mayhewlabs.com/), modificada para que funcione con este shield, para la lectura de las entradas y salidas del shield
- [NewPing] (http://playground.arduino.cc/Code/NewPing) para el manejo del sensor de ultrasonido
- [Arduino MIDI Library v4.2] (https://github.com/FortySevenEffects/arduino_midi_library/releases/tag/4.2), para el manejo de la comunicación MIDI

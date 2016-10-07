En esta carpeta se halla el código necesario (librerías y sketch) para usar con la versión 2 del KiloMux Shield (la que parece de verdad).

El sketch <b><i>v4midiController.ino</i></b> hace uso de las siguientes librerías:
- [Kilomux Shield] (https://github.com/Yaeltex/Kilomux-shield/blob/master/Arduino%20Code/KilomuxShield%20Library/KilomuxShieldLib.zip?raw=true) para Arduino, para la lectura de las entradas y salidas del shield
- [KilomuxSysEx] (https://github.com/Yaeltex/Kilomux-shield/tree/master/Arduino%20Code/v4_MIDI_controller/Librer%C3%ADas/KilomuxSysEx) que contiene los métodos necesarios para leer los datos de configuración de la EEPROM.
- [NewPing] (http://playground.arduino.cc/Code/NewPing) para el manejo del sensor de ultrasonido
- [Arduino MIDI Library v4.2] (https://github.com/FortySevenEffects/arduino_midi_library/releases/tag/4.2), para el manejo de la comunicación MIDI

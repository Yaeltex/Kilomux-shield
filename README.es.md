# Kilomux Shield
Repositorio para el Kilomux Shield para Arduino desarrollado en Yaeltex
con colaboración de Jorge Crowe, del Laboratorio del Juguete, ambos
ubicados en Buenos Aires, Argentina.

El KiloMux es un shield para Arduino UNO que expande la cantidad de
entradas y salidas usando multiplexores e integrados 74HC595 y permite
la comunicación via MIDI, ya sea por USB o por hardware.

Se tienen 16 salidas digitales (por ahora sin PWM) y 32 entradas analógicas
o digitales, a voluntad. 

El shield tiene también 4 pines digitales libres (2, 3, 12 y 13 en la v1 y 
10, 11, 12 y 13 en la v2), pensado para conectar un sensor de ultrasonido 
y un botón y un LED para su activación.

En el repositorio se encuentra el código de Arduino que realiza la
lectura de las entradas, con funciones para enviar los datos usando la
librería MIDI de Arduino, o por serial y que recibe datos por MIDI o
serial y enciende las salidas correspondientes.

Se pueden encontrar las librerías de código necesarias para el
funcionamiento del sketch, que son la MuxShield (modificada de la de
MayhewLabs), MIDI (Arduino) y NewPing (para el sensor de ultrasonido).

También se encuentra en el repositorio entero de HIDUINO, el firmware
necesario para envíar MIDI por USB y que la PC reconozca a la Arduino
como un dispositivo MIDI. En la carpeta "hiduino-master" se puede
encontrar un tutorial para recompilar (y renombrar) el firmware y para
cargarlo en la Arduino.

Para más información acerca del shield, visitá [nuestra wiki](https://wiki.yaeltex.com/?nocache=1#.C2.B7_Kilomux_Shield)

Si tenes dudas, sugerencias o comentarios acerca del shield, escribí a
franco@yaeltex.com o posteala en el [foro de Yaeltex](http://forum.yaeltex.com)

Saludos!

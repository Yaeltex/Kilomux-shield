# Kilomux Shield
This is the Kilomux Arduino Shield repository, developed by Yaeltex in collaboration with Jorge Crowe from El Laboratorio del Juguete, both located in Buenos Aires, Argentina.

Kilomux is an Arduino UNO shield which expands the number of inputs and outputs using multiplexers and 74HC595 ICs and allows MIDI communication via USB or hardware.

It has 16 digital outputs (without PWM for the moment) and 32 analog/digital inputs, which can be used independently with the provided functions. 
The shield has also 4 free digital pins (2, 3, 12 and 13 in v1, and 10, 11, 12 and 13 in v2), intended to connect an ultrasonic sensor, a button and an LED for its activation.

In the repository you can find the [Kilomux Arduino Library](Arduino%20Code/KilomuxShield%20Library/KilomuxShieldLib.zip?raw=true) with the basic methods to read inputs and drive outputs, [examples of use] (Arduino%20Code/KilomuxShield%20Library/examples), and [templates for MIDI controllers] ().

The code libraries necessary for the sketch functioning are also available: the MuxShield (a mod of MayhewLabs' library), MIDI (Arduino) and NewPing (for the ultrasonic sensor).

You can also find the whole HIDUINO repository - the firmware to send MIDI via USB and allow PC to recognize the Arduino as a MIDI device. In the "hiduino-master" folder you can find a tutorial on how to recompile (and rename) the firmware and load it into the Arduino device.

For more info, go to our [wiki](http://wiki.yaeltex.com.ar/index.php?title=KiloMux_Shield) (in Spanish, for now)

If you have any questions, suggestions or comments about the shield, email us at franco@yaeltex.com.ar or post in Yaeltex's forum http://foro.yaeltex.com.ar

Cheers!


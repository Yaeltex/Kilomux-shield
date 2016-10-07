#include "Comms.h"


#if defined(MIDI_COMMS)
// Lee el canal midi, note y velocity, y actualiza el estado de los leds.
void ReadMidi(void) {
  // COMENTAR PARA DEBUGGEAR CON SERIAL ///////////////////////////////////////////////////////////////////////
  if (MIDI.read()) {         // ¿Llegó un mensaje MIDI?
    if(MIDI.getType() == midi::SystemExclusive){
      int sysexLength = 0;
        
      sysexLength = (int) MIDI.getSysExArrayLength();
      pMsg = MIDI.getSysExArray();
        
      char sysexID[3]; 
      sysexID[0] = (char) pMsg[1]; 
      sysexID[1] = (char) pMsg[2]; 
      sysexID[2] = (char) pMsg[3];
        
      char command = pMsg[4];
        
      if(sysexID[0] == 'Y' && sysexID[1] == 'T' && sysexID[2] == 'X'){
          
        if (command == 1){            // Enter config mode
          flagBlink = 1;
          blinkCount = 1;
          configMode = 1;
        }                             
         
        if(command == 3 && configMode){             // Save dump data
          if (!receivingSysEx){
            receivingSysEx = 1;
            MIDI.sendNoteOn(127, receivingSysEx,1);
          }
          
          KMS::io.write(packetSize*pMsg[5], pMsg+6, sysexLength-7);      // PACKET_SIZE is 200, pMsg has index in byte 6, total sysex packet has max. 207 bytes 
                                                                          //                                                |F0, Y , T , X, command, index, F7|
          MIDI.sendNoteOn(127, pMsg[5],1);                                                                          
          flagBlink = 1;                                              
          blinkCount = 2;
          
          if (sysexLength < 207){   // Last message?
            receivingSysEx = 0;
            MIDI.sendNoteOn(127, receivingSysEx,1);
            flagBlink = 1;
            blinkCount = 5;
          }  
        }  
      }
    } 
  }
  // FIN COMENTARIO ///////////////////////////////////////////////////////////////////////////////////////////
}
#endif

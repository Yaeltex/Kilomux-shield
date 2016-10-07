#ifndef _COMMS_H_
#define _COMMS_H_

#include <Kilomux.h>
#include <KM_Data.h>
#include <MIDI.h>
#include <midi_Defs.h>
#include <midi_Message.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>

#define MIDI_COMMS

#if defined(MIDI_COMMS)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Descomentar la próxima línea si el compilador no encuentra MIDI
MIDI_CREATE_DEFAULT_INSTANCE()
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

const byte *pMsg;
extern bool flagBlink, configMode, receivingSysEx;
extern byte blinkCount;
extern unsigned int packetSize;

void ReadMidi(void);
#endif

#endif

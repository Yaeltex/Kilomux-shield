
/* * * * * * * * * * * * * * * * * * * * * * *
* Code by Martin Sebastian Wain for YAELTEX *
* contact@2bam.com                     2016 *
* * * * * * * * * * * * * * * * * * * * * * */

#ifndef _KM_DATA_H_
#define _KM_DATA_H_

#include <Arduino.h>
#include "KM_Accessors.h"
#include "KM_EEPROM.h"

#if 1
  #define DEBUG_PRINT(lbl, data) { Serial.print(lbl); Serial.print(": "); Serial.println(data); }
#else
  #define DEBUG_PRINT(lbl, data)
#endif

namespace KMS {
  enum Mode {
    M_OFF = 0,
    M_NOTE = 1,
    M_CC = 2,
    M_NRPN = 3,
    // Only for inputs (save one bit in outputs)
    M_SHIFTER = 4
  };
  enum Type {
    T_DIGITAL = 0,
    T_ANALOG = 1
  };

  static const byte PROTOCOL_VERSION = 1;
  extern EEPROM_IO io;

  namespace priv {
    extern int _inputsOffset;
    extern int _outputsOffset;
    extern byte _currBank;
    extern byte _maxBank;
    extern int _bankLen;
    extern int _bankOffset;
  }

  //This function should be called before all operations to KMS namespace.
  void initialize();
  
  //Returns number of banks. This is better than GlobalData::numBanks() because the user might have misconfigured using a lower memory device than the config.
  inline byte realBanks() { return priv::_maxBank; }
  inline GlobalData globalData() { return GlobalData(); }  //This is superfluous but keeps the interface
  inline InputUS ultrasound() { return InputUS(priv::_bankOffset); }
  inline InputNorm input(int index) { return InputNorm(priv::_bankOffset + priv::_inputsOffset + InputNorm::length * index); }
  inline Output output(int index) { return Output(priv::_bankOffset + priv::_outputsOffset + Output::length * index); }

  inline byte bank() { return priv::_currBank; }
  void setBank(byte bank);

} //namespace KMS

#endif // _KM_DATA_H_


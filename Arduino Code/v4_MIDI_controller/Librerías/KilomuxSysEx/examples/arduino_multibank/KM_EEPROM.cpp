
/* * * * * * * * * * * * * * * * * * * * * * *
* Code by Martin Sebastian Wain for YAELTEX *
* contact@2bam.com                     2016 *
* * * * * * * * * * * * * * * * * * * * * * */

#include "KM_EEPROM.h"
#include "EEPROMex.h"

namespace KMS {

  void EEPROM_IO::read(unsigned int address, byte *buf, int len) {
    address += KMS_DATA_START;
    while(len--)
      *(buf++) = EEPROM.readByte(address++);
  }
  void EEPROM_IO::write(unsigned int address, const byte *buf, int len) {
    address += KMS_DATA_START;
    while(len--)
      EEPROM.updateByte(address++, *(buf++));   //Update only writes if the value changed (extends EEPROM lifetime)
  }
  unsigned int EEPROM_IO::length(void) {
	#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
	return EEPROMSizeMega;
	#elif defined(__AVR_ATmega328P__)
	return EEPROMSizeUno;
	#endif
  }
  bool EEPROM_IO::isReady() {
    return EEPROM.isReady();
  }
}

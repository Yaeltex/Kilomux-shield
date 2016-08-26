
/* * * * * * * * * * * * * * * * * * * * * * *
* Code by Martin Sebastian Wain for YAELTEX *
* contact@2bam.com                     2016 *
* * * * * * * * * * * * * * * * * * * * * * */

#include "KM_EEPROM.h"
#include <EEPROM.h>

namespace KMS {

	void EEPROM_IO::read(unsigned int address, byte *buf, int len) {
		address += KMS_DATA_START;
		while(len--)
			*(buf++) = EEPROM.read(address++);
	}
	void EEPROM_IO::write(unsigned int address, const byte *buf, int len) {
		address += KMS_DATA_START;
		while(len--)
			EEPROM.update(address++, *(buf++));		//Update only writes if the value changed (extends EEPROM lifetime)
	}
	unsigned int EEPROM_IO::length(void) {
		return EEPROM.length();
	}
}
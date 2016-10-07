
/* * * * * * * * * * * * * * * * * * * * * * *
* Code by Martin Sebastian Wain for YAELTEX *
* contact@2bam.com                     2016 *
* * * * * * * * * * * * * * * * * * * * * * */

#ifndef _KM_EEPROM_H_
#define _KM_EEPROM_H_

//User set configuration for limits
#define KMS_DATA_START      0     
#define KMS_MAX_DATA      1024

#include <Arduino.h>

namespace KMS {
  //Structure with defined interface, expandable for future replacement
  class EEPROM_IO {
  public:
    //Read from the EEPROM
    //Address is 0-based and automatically offseted by KMS_EEPROM_START
    void read(unsigned int address, byte *buf, int len);

    //Write to the EEPROM
    //Address is 0-based and automatically offseted by KMS_EEPROM_START
    void write(unsigned int address, const byte *buf, int len);
    
    //Get the EEPROM length in bytes
    unsigned int length(void);
	
	//Check if EEPROM is ready for a new write cycle
    bool isReady(void);
  };

} //namespace KMS

#endif // _KM_EEPROM_H_


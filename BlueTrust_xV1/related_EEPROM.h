/*
related_EEPROM.h


EEPROM DATA ADDRESS
[ADDRESS] : [DATA INFO]
-----------------------
0번지~5번지: (컴파일된 연,월,일,시,분,초)

*/
#ifndef _DEFINED_EEPROM_ADDRESSES_
#define _DEFINED_EEPROM_ADDRESSES_
#define ADDR_COMPILE_TIME_YEAR 0
#define ADDR_COMPILE_TIME_MONTH 1
#define ADDR_COMPILE_TIME_DAY 2
#define ADDR_COMPILE_TIME_HOUR 3
#define ADDR_COMPILE_TIME_MINUTE 4
#define ADDR_COMPILE_TIME_SECOND 5
#define EEPROM_ADDRESS_AMOUNT 6 // EEPROM에 저장된 개수


// Functions:
uint8_t ReadRom(unsigned int addr);
void WriteRom(unsigned int addr, uint8_t value);
#endif
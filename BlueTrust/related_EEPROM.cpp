/*
related_EEPROM.cpp
*/
// Functions:
#include <EEPROM.h>
uint8_t ReadRom(unsigned int addr){
    return EEPROM.read(addr);
}
void WriteRom(unsigned int addr, uint8_t value){
    EEPROM.write(addr, value);
}
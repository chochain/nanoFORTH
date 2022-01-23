/**
 * @file mockrom.h
 * @brief nanoForth mock EEPROM interface class (for testing)
 */
#ifndef __SRC_EEPROM_H
#define __SRC_EEPROM_H

#if !ARDUINO
static U8 _eeprom[N4_MEM_SZ];          ///< mock EEPROM storage
class MockRom                          ///< mock EEPROM access class
{
public:
    U16  length() { return N4_MEM_SZ; }
    U8   read(U16 idx) { return _eeprom[idx]; }
    void update(U16 idx, U8 v) { _eeprom[idx] = v; }
};

MockRom EEPROM;                        ///< mock EEPROM access object instance
#endif // ARDUINO
#endif // __SRC_MOCKROM_H

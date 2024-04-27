/**
 * @file
 * @brief nanoForth mock EEPROM interface class (for testing)
 */
#ifndef __SRC_EEPROM_H
#define __SRC_EEPROM_H

#define EEPROM_SZ 0x400                /* default 1K */

#if !ARDUINO
static U8 _eeprom[EEPROM_SZ];          ///< mock EEPROM storage
class MockRom                          ///< mock EEPROM access class
{
public:
    IU   length() { return EEPROM_SZ; }
    U8   read(IU idx) { return _eeprom[idx]; }
    void update(IU idx, U8 v) { _eeprom[idx] = v; }
};

MockRom EEPROM;                        ///< mock EEPROM access object instance
#endif // ARDUINO
#endif // __SRC_MOCKROM_H

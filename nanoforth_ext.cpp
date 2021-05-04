//
// nanoForth extended and arduino specific words
//
#include <Arduino.h>
#include <EEPROM.h>
#include "nanoforth.h"

extern void list_words();

void _words()
{
    U8 n = 0;
    for (U8 *p=last; p!=PTR(0xffff); p=PTR(GET16(p)), n++) {
        if (n%10==0) d_chr('\n');
#if EXE_TRACE
        d_adr(IDX(p)); d_chr(':');                            // optionally show address
#endif // EXE_TRACE
        d_chr(p[2]); d_chr(p[3]); d_chr(p[4]); d_chr(' ');    // 3-char name + space
    }
}

void _save()
{
    U16 last0 = IDX(last), here0 = IDX(here);
    EEPROM.update(0, last0>>8); EEPROM.update(1, last0&0xff);
    EEPROM.update(2, here0>>8); EEPROM.update(3, here0&0xff);
    U8 *p = dic;
    for (int i=0; i<here0; i++) {
        EEPROM.update(i+4, *p++);
    }
}

void _load()
{
    U16 last0 = ((U16)EEPROM.read(0)<<8) + EEPROM.read(1);
    U16 here0 = ((U16)EEPROM.read(2)<<8) + EEPROM.read(3);
    U8 *p = dic;                              // starting of RAM dictionary
    for (int i=0; i<here0; i++) {
        *p++ = EEPROM.read(i+4);
    }
    last = PTR(last0);                        // reset pointer to last word
    here = PTR(here0);                        // reset pointer to top of dictionary
}

void extended(U8 op)
{
    switch (op) {
    case 0:  PUSH(IDX(here));              break; // HRE
    case 1:  PUSH(IDX(last));              break; // CP
    case 2:  PUSH(TOS1);                   break; // OVR
    case 3:  TOS = -TOS;                   break; // INV
    case 4:  PUSH(POP()*sizeof(U16));      break; // CEL
    case 5:  here += POP();                break; // ALO
    case 6:  _words(); list_words();       break; // WRD
    case 7:  _save();                      break; // SAV
    case 8:  _load();                      break; // LD
    case 9:  vm_delay(POP());              break; // DLY
    case 10: PUSH(digitalRead(POP()));     break; // IN
    case 11: digitalWrite(POP(), POP());   break; // OUT
    case 12: PUSH(analogRead(POP()));      break; // AIN
    }
}


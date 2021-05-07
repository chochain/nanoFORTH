#ifndef __SRC_NANOFORTH_H
#define __SRC_NANOFORTH_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <Arduino.h>

#define STK_SZ       0x80
#define DIC_SZ       0x400
#define MEM_SZ       (DIC_SZ+STK_SZ)

typedef uint8_t      U8;
typedef uint16_t     U16;
typedef int16_t      S16;
typedef uint32_t     U32;
//
// parser flag
//
enum {
    TKN_EXE = 1,
    TKN_DIC,
    TKN_EXT,
    TKN_PRM,
    TKN_NUM,
    TKN_ERR
};
//
// Serial IO macros
//
#define putstr(msg)    Serial.print(F(msg))
#define putchr(c)      Serial.write((char)c)
#define puthex(v)      Serial.print((U16)v, HEX)
//
// dictionary index <=> pointer translation macros
//
#define PTR(n)         ((U8*)dic + (n))
#define IDX(p)         ((U16)((U8*)(p) - dic))
//
// memory access opcodes
//
#define SET8(p, c)     (*(U8*)(p)++=(U8)(c))
#define SET16(p, n)    do { U16 x=(U16)(n); SET8(p,(x)>>8); SET8(p,(x)&0xff); } while(0)
#define GET16(p)       (((U16)(*(U8*)(p))<<8) + *((U8*)(p)+1))

extern void n4_loop();            // virtual function
extern int  n4_delay(U32 ms);     // hardware yield to NanoForth

class N4VM;
class NanoForth
{
    N4VM *vm;
    
public:
    NanoForth();
    
    bool run();
    //
    // multi-threaded VM functions (prefixed n4 to avoid collision with Arduino.h)
    //
    static char key();                // Arduino's getchar()
    static void yield();              // NanoForth yield to hardware context
    static void wait(U32 ms);         // NanoForth thread delay
};

#endif // __SRC_NANOFORTH_H

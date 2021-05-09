#ifndef __SRC_NANOFORTH_H
#define __SRC_NANOFORTH_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <Arduino.h>
#include <pt.h>

#define ASM_TRACE      1                /* enable assembler tracing    */
#define EXE_TRACE      1                /* enable VM execution tracing */

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
/* for debugging
#define PROGMEM
#define pgm_read_byte(p)  (*(p))
#define millis()          0
#define putstr(msg)    printf("%s", msg)
#define putchr(c)      printf("%c", c)
#define puthex(v)      printf("%02x", v)
*/
#define putstr(msg)    Serial.print(F(msg))
#define putchr(c)      Serial.write((char)c)
#define puthex(v)      Serial.print((U16)v, HEX)
//
// memory access opcodes
//
#define SET8(p, c)     (*(U8*)(p)++=(U8)(c))
#define SET16(p, n)    do { U16 x=(U16)(n); SET8(p,(x)>>8); SET8(p,(x)&0xff); } while(0)
#define GET16(p)       (((U16)(*(U8*)(p))<<8) + *((U8*)(p)+1))
//
// light-weight multi-tasker (aka protothread by Adam Dunkels)
//
typedef struct n4_t {
    void (*func)(n4_t*); // function pointer
    n4_t *next;          // next item in linked-list (root=0)
    U32  t;              // delay timer
    U16  ci;             // protothread case index
} n4_task;               

#define N4_FUNC(fname)  void fname(n4_task *_p_)
#define N4_BEGIN()      switch((_p_)->ci) { case 0:
#define N4_DELAY(ms)    (_p_)->t = millis()+(U32)(ms); (_p_)->ci = __LINE__; case __LINE__: \
                        if (millis() < (_p_)->t) return;
#define N4_END()        } (_p_)->ci = 0;
//
// NanoForth control object
//
class N4VM;
class NanoForth
{
    N4VM *vm;
    
public:
    NanoForth();
    NanoForth(U16 mem_sz, U16 stk_sz);

    void add(void (*ufunc)(n4_task*));   // add the user function to NanoForth task manager
    bool run();                          // run one NanoForth VM cycle (and some CPU to user tasks)
    //
    // multi-threaded VM functions (prefixed n4 to avoid collision with Arduino.h)
    //
    static void yield();                 // NanoForth yield to hardware context
    static char key();                   // Arduino's getchar(), yield to hardware context when waiting
    static void wait(U32 ms);            // NanoForth thread delay(), yield to hardware context
};

#endif // __SRC_NANOFORTH_H

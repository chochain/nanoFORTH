///
/// \file nanoforth.h
/// \brief NanoForth main controller
///
///> `lib objects..[dictionary->...|sp-> ..stack.. <-rp],vm,asm ..free.. <-heap|`
///
#ifndef __SRC_NANOFORTH_H
#define __SRC_NANOFORTH_H
#if ARDUINO
#include <Arduino.h>
#else
#include <cstdint>                            // uint_t
#include <cstdio>                             // printf
#include <cstdlib>                            // malloc
#include <iostream>
#define PROGMEM
#define millis()          10000
#define pgm_read_byte(p)  (*(p))
extern int Serial;
#define Stream            int
#endif // ARDUINO
//
// commonly used portable types
//
typedef uint8_t      U8;                         ///< 8-bit unsigned integer, for char and short int
typedef uint16_t     U16;                        ///< 16-bit unsigned integer, for return stack, and pointers
typedef int16_t      S16;                        ///< 16-bit signed integer, for general numbers
typedef uint32_t     U32;                        ///< 32-bit unsigned integer, for millis()
typedef int32_t      S32;                        ///< 32-bit signed integer
//
// default heap sizing
//
constexpr U16 N4_STK_SZ = 0x80;                  /**< default parameter/return stack size       */
constexpr U16 N4_DIC_SZ = 0x400;                 /**< default dictionary size                   */
constexpr U16 N4_MEM_SZ = (N4_DIC_SZ+N4_STK_SZ); /**< total memory block allocate for NanoForth */

/// NanoForth light-weight multi-tasker (aka protothread by Adam Dunkels)
///
typedef struct n4_task {    
    void (*func)(n4_task*);                   ///< function pointer
    n4_task *next;                            ///< next item in linked-list (root=0)
    U32  t;                                   ///< delay timer
    U16  ci;                                  ///< protothread case index
} *n4_tptr;
//
// NanoForth multi-tasking macros
//
/// \def N4_TASK
/// \brief define a user task block with name
/// \def N4_DELAY
/// \brief pause the user task for ms microseconds
/// \def N4_END
/// \brief end of the user task block.
///
#define N4_TASK(tname)  void tname(n4_tptr _p_) { switch((_p_)->ci) { case 0:
#define N4_DELAY(ms)    (_p_)->t = millis()+(U32)(ms); (_p_)->ci = __LINE__; case __LINE__: \
                        if (millis() < (_p_)->t) return;
#define N4_END          } (_p_)->ci = 0; }
///
/// NanoForth main control object (with static members that support multi-threading)
///
class N4VM;
class NanoForth
{
    static n4_tptr _n4tsk;        ///< user function linked-list
    
    U8     *_mem;                 ///< pointer to nanoForth memory block
    N4VM   *_n4vm;                ///< virtual machine object pointer

public:
    ///
    /// initializer with dynamic memory sizing (return 1 if allocation failed)
    ///
    int  begin(
        Stream &io=Serial,        ///< iostream which can be redirected to SoftwareSerial
        U16 mem_sz=N4_MEM_SZ,     ///< memory size (default: N4_MEM_SZ=0x480)
        U16 stk_sz=N4_STK_SZ      ///< parameter+return stack size (default: N4_STK_SZ=0x80)
        );                        ///< placeholder for extra setup
    void exec();                  ///< NanoForth run one line of command input
    //
    // protothreading support
    //
    static void add(void (*ufunc)(n4_tptr));  ///< add the user function to NanoForth task manager
    static void yield();          ///< NanoForth yield to user tasks
    static void wait(U32 ms);     ///< pause NanoForth thread for ms microseconds, yield to user tasks
};
///
/// short-hand to NanoForth class
///
typedef NanoForth N4;

#endif // __SRC_NANOFORTH_H

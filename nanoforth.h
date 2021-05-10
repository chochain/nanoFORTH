///
/// \file nanoforth.h
/// \brief NanoForth main controller
///
///> `lib objects..[dictionary->...|sp-> ..stack.. <-rp],vm,asm ..free.. <-heap|`
///
#ifndef __SRC_NANOFORTH_H
#define __SRC_NANOFORTH_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <Arduino.h>

#define ASM_TRACE    1                        /**< enable assembler tracing    */
#define EXE_TRACE    1                        /**< enable VM execution tracing */
//
// default heap sizing
//
#define N4_STK_SZ    0x80                     /**< default parameter/return stack size       */
#define N4_DIC_SZ    0x400                    /**< default dictionary size                   */
#define N4_MEM_SZ    (N4_DIC_SZ+N4_STK_SZ)    /**< total memory block allocate for NanoForth */
//
// commonly used portable types
//
typedef uint8_t      U8;                      ///< 8-bit unsigned integer, for char and short int
typedef uint16_t     U16;                     ///< 16-bit unsigned integer, for return stack, and pointers
typedef int16_t      S16;                     ///< 16-bit signed integer, for general numbers
typedef uint32_t     U32;                     ///< 32-bit unsigned integer, for millis()

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
/// \def N4_FUNC
/// \brief define a user function.
/// \def N4_BEGIN
/// \brief begin of the user function block.
/// \def N4_DELAY
/// \brief pause the user function for ms microseconds
/// \def N4_END
/// \brief end of the user function block.
///
#define N4_FUNC(fname)  void fname(n4_tptr _p_)
#define N4_BEGIN()      switch((_p_)->ci) { case 0:
#define N4_DELAY(ms)    (_p_)->t = millis()+(U32)(ms); (_p_)->ci = __LINE__; case __LINE__: \
                        if (millis() < (_p_)->t) return;
#define N4_END()        } (_p_)->ci = 0;
///
/// NanoForth main control object (with static members that support multi-threading)
///
class NanoForth
{
public:
    /// constructor with dynamic memory sizing
    static void begin(
        unsigned int mem_sz=N4_MEM_SZ,  ///< memory size
        unsigned int stk_sz=N4_STK_SZ   ///< parameter+return stack size
        );   
    
    static void add(void (*ufunc)(n4_tptr));  ///< add the user function to NanoForth task manager
    static bool step();                       ///< run one NanoForth VM cycle, and to each of user tasks
    
    static void yield();          ///< NanoForth yield to user tasks
    static char key();            ///< Arduino's getchar(), yield to user tasks when waiting
    static void wait(U32 ms);     ///< pause NanoForth thread for ms microseconds, yield to user tasks
};
///
/// short hand to NanoForth class
///
typedef NanoForth N4;

#endif // __SRC_NANOFORTH_H

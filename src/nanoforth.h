/**
 * @file nanoforth.h
 * @brief nanoForth main controller
 *
 * ####Memory Map
 *
 *    |paddr |forth |objects           |rom|
 *    |-----:|-----:|:----------------:|:-:|
 *    |0x0000|      |Interrupt Vectors |   |
 *    |0x0100|      |Arduino libraries |   |
 *    |      |0x0000|Dictionary==>     |x  |
 *    |      |      |...1K-byte...     |x  |
 *    |      |0x0400|Return Stack==>   |   |
 *    |      |      |...128 entries... |   |
 *    |      |      |<==Data Stack     |   |
 *    |      |0x0500|Input Buffer      |   |
 *    |      |      |...free memory... |   |
 *    |      |      |Arduino heap      |   |
 *    |0x0900|      |                  |   |
 */
#ifndef __SRC_NANOFORTH_H
#define __SRC_NANOFORTH_H

#define TRC_LEVEL         1       /* tracing verbosity level        */
#define ISR_PERIOD        16      /* tick divider, higher => longer */

///@name Arduino Console Output Support
///@{
#if ARDUINO
#include <Arduino.h>
#define log(msg)          Serial.print(F(msg))
#define logx(v)           Serial.print((U16)v, HEX)
#else
#include <cstdint>                // uint_t
#include <cstdio>                 // printf
#include <cstdlib>                // malloc
#include <iostream>
#define PROGMEM
#define millis()          10000
#define pgm_read_byte(p)  (*(p))
#define log(msg)          ::printf("%s", msg)
#define logx(v)           ::printf("%x", (U16)v)
#define Stream            int
extern  int Serial;
#endif // ARDUINO
#define INLINE            inline __attribute__((always_inline))

///@}
///
///@name Portable Types
///@{
typedef uint8_t      U8;          ///< 8-bit unsigned integer, for char and short int
typedef uint16_t     U16;         ///< 16-bit unsigned integer, for return stack, and pointers
typedef int16_t      S16;         ///< 16-bit signed integer, for general numbers
typedef uint32_t     U32;         ///< 32-bit unsigned integer, for millis()
typedef int32_t      S32;         ///< 32-bit signed integer
///@}
///
//@name Default Heap sizing
///@{
constexpr U16 N4_DIC_SZ = 0x400;                 /**< default dictionary size                   */
constexpr U16 N4_STK_SZ = 0x100;                 /**< default parameter/return stack size       */
constexpr U16 N4_TIB_SZ = 0x100;                 /**< default terminal input buffer size        */
///@}
///
/// nanoForth light-weight multi-tasker (aka protothread by Adam Dunkels)
///
typedef struct n4_task {
    void (*func)(n4_task*);       ///< function pointer
    n4_task *next;                ///< next item in linked-list (root=0)
    U32  t;                       ///< delay timer
    U16  ci;                      ///< protothread case index
} *n4_tptr;
///
///@name nanoForth multi-tasking
///
/// \def N4_TASK
/// \brief define a user task block with name
/// \def N4_DELAY
/// \brief pause the user task for ms microseconds
/// \def N4_END
/// \brief end of the user task block.
///
///@{
#define N4_TASK(tname)  void tname(n4_tptr _p_) { switch((_p_)->ci) { case 0:
#define N4_DELAY(ms)    (_p_)->t = millis()+(U32)(ms); (_p_)->ci = __LINE__; case __LINE__: \
                        if (millis() < (_p_)->t) return;
#define N4_END          } (_p_)->ci = 0; }
///@}
///
/// nanoForth main control object (with static members that support multi-threading)
///
class NanoForth
{
    static n4_tptr _n4tsk;        ///< user function linked-list

    U8     *_mem;                 ///< pointer to nanoForth memory block

public:
    ///
    /// initializer with dynamic memory sizing
    ///
    int  begin(
        Stream &io=Serial,        ///< iostream which can be redirected to SoftwareSerial
        U8  ucase=1,              ///< case sensitiveness (default: insensitive)
        U16 dic_sz=N4_DIC_SZ,     ///< dictionary size (default: N4_DIC_SZ=0x400)
        U16 stk_sz=N4_STK_SZ,     ///< parameter+return stack size (default: N4_STK_SZ=0x80)
		U16 tib_sz=N4_TIB_SZ
        );                        ///< placeholder for extra setup
    void exec();                  ///< nanoForth run one line of command input
    //
    // protothreading support
    //
    static void add(              ///< add the user function to NanoForth task manager
        void (*ufunc)(n4_tptr)    ///< user task pointer to be added
        );
    static void yield();          ///< nanoForth yield to user tasks
    static void wait(U32 ms);     ///< pause NanoForth thread for ms microseconds, yield to user tasks
};
///
/// short-hand to NanoForth class
///
typedef NanoForth N4;

#endif // __SRC_NANOFORTH_H

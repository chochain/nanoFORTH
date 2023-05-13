/**
 * @file
 * @brief nanoForth main controller
 *
 * ####Memory Map
 *
 *    |paddr |forth |objects           |rom|
 *    |-----:|-----:|:----------------:|:-:|
 *    |0x0000|      |Interrupt Vectors |   |
 *    |0x0100|      |Arduino libraries |   |
 *    |0x02c0|0x0000|Dictionary==>     |x  |
 *    |      |      |...1K-byte...     |x  |
 *    |0x06c0|0x0400|Return Stack==>   |   |
 *    |      |      |...64 entries...  |   |
 *    |      |      |<==Data Stack     |   |
 *    |0x0740|0x0480|Input Buffer      |   |
 *    |      |      |...free memory... |   |
 *    |      |      |Arduino heap      |   |
 *    |0x0900|      |                  |   |
 */
#ifndef __SRC_N4_H
#define __SRC_N4_H

#define APP_NAME          "\nnanoForth "
#define APP_VERSION       "2.0 "
#define N4_API_SZ         8       /**< C API function pointer slots */
#define TRC_LEVEL         0       /**< tracing verbosity level      */

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
#define random(v)         (rand()%v)
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
typedef void (*FPTR)();           ///< function pointer
///@}
///
/// nanoForth main control object (with static members that support multi-threading)
///
class NanoForth
{
	static FPTR fp[N4_API_SZ];    ///< C API function pointer slots

public:
    void setup(
    	const char *code=0,       ///< preload Forth code
        Stream &io=Serial,        ///< iostream which can be redirected to SoftwareSerial
        U8 ucase=0                ///< case sensitiveness (default: sensitive)
        );                        ///< placeholder for extra setup
    void exec();                  ///< nanoForth execute one line of command input
    //
    // protothreading support
    //
    static void add_api(          ///< add the user function to NanoForth task manager
    	int  i,                   ///< index of function pointer slots
        void (*fp)()              ///< user function pointer to be added
        );
    static void yield();          ///< nanoForth yield to user tasks
    static void wait(U32 ms);     ///< pause NanoForth thread for ms microseconds, yield to user tasks
    static void call_api(U16 id); ///< call C API interface
};

#endif // __SRC_N4_H

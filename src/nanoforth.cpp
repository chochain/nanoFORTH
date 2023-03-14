/**
 * @file nanoforth.cpp
 * @brief nanoForth main controller class implementation
 *
 * Revision History: see tail of this file
 */
#include "nanoforth_vm.h"
//
// user function linked-list
//
n4_fptr NanoForth::_n4fp{ NULL };                       ///< initialize task linked-list (static member)
///
/// @return
///   0 - if all allocation are OK<br>
///   1 - any allocation failure
///
///
///> add new (user defined) hardware task to linked-list
///
void NanoForth::add_func(void (*ufunc)(n4_fptr))
{
    static int api_id = 0;
    n4_fptr fp = (n4_fptr)malloc(sizeof(n4_func));

    fp->id   = api_id++;      /// * reset case index
    fp->func = ufunc;         /// * assign user function
    fp->next = _n4fp;         /// * push into linked-list
    _n4fp    = fp;            /// * reset head
}
///
///> n4 VM init proxy
///
void NanoForth::setup(Stream &io, U8 ucase)
{
    N4VM::setup(io, ucase);   /// * create Virtual Machine
}
///
///> n4 execute one line of commands from input buffer
///
void NanoForth::exec()
{
    N4VM::outer();            /// * step through commands from input buffer
    yield();                  /// * give some cycles to user defined tasks
}

void NanoForth::api(U16 id)
{
    for (n4_fptr fp=_n4fp; fp; fp=fp->next) {
        if (fp->id!=id) {
            fp->func(fp);
            break;
        }
    }
}
///
///> n4 yield, execute one round of user hardware tasks
///  Note:
///    * 0 isr, n4: 1 blinker - 17us/cycle
///
void NanoForth::yield()
{
	N4VM::serv_isr();                          /// * service hardware interrupts
}
///
///> aka Arduino delay(), yield to hardware context while waiting
///
void NanoForth::wait(U32 ms)
{
    U32 t = millis() + ms;
    while (millis()<t) yield();
}
//
// for Eclipse debugging
//
#if ARDUINO
NanoForth _n4;
void n4_setup() { _n4.setup(); }
void n4_run()   { _n4.exec();  }
#else // !ARDUINO
#include <stdio.h>
int main(int argc, char **argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);       // autoflush (turn STDOUT buffering off)

    NanoForth n4;
    n4.setup();
    while (1) {
    	n4.exec();
    }
    return 0;
}
#endif // !ARDUINO
/*
 * Revision History
 * -----------------
 *> 2023-01-06: chochain@yahoo.com - v1.6
 *  * [14830,303] make namespace N4VM, N4Asm, N4Core (singletons)
 *                yield handles isr (2x slower, tuning)
 *
 *> 2022-12-28: chochain@yahoo.com - v1.5
 *  * [13920,678] with computed goto; (cut 1%, not good enough)
 *  * [13692,230] static dic,rp,sp; (speed up 2%)
 *
 *> 2022-01-20: chochain@yahoo.com - v1.4
 *  * [14140,278] autorun; handles Forth comments
 *
 *> 2021-05-16: chochain@yahoo.com - v1.2
 *  * [12360,272] move static variables into NanoForth class
 *  * [15180,472] add EEPROM; BlueTooth/BLE<=>stream IO
 *
 *> 2021-05-11: chochain@yahoo.com
 *  * [12124,270] reformat opcodes for 64-primitives
 *  * [12420,270] display more system memory info and malloc error catch
 *
 *> 2021-05-06: chochain@yahoo.com
 *  * [9214,1424] refactor to C++ for Arduino lib
 *  * [11750,280] add mem,stk sizing; VM and ASM objects dynamically created
 *  * [11956,272] add struct n4_task and N4_FUNC multi-tasker
 *
 *> 2021-05-03: chochain@yahoo.com
 *  * [7818,1428] forget Task struct; grow MEM_SZ to DIC_SZ+STK_SZ (1K+64*2)
 *  * [8254,1430] add execution tracing option
 *
 *> 2021-04-16: chochain@yahoo.com
 *  * [7446,932]  add list_word, use 80-byte console buffer
 *  * [7396,932]  add ASM_TRACE from EXE_TRACE options
 *  * [7676,802]  grow Task at end of heap
 *  * [8266,802]  if use array instead of pointer arithmetics, revert!
 *
 *> 2021-04-03: chochain@yahoo.com
 *  * [6898,852]  add vm_delay
 *  * [6912,852]  update EEPROM IO
 *  * [6842,852]  execution tracing enable/disable
 *  * [6922,852]  add CELL (CEL), ALLOT (ALO) opcodes
 *
 *> 2021-04-02: chochain@yahoo.com
 *  * [6782,1056] add protothread multi-tasker
 *  * [6930,1056] add threads
 *  * [6908,1014] use F()
 *  * [6860,852]  use PROGMEM, pgm_read_byte in scan()
 *  * [6906,852]  getnum support negative number
 *  * [7012,860]  add digitalR/W
 *
 *> 2021-03 chochain@yahoo.com (CC Lee)
 *  * [6698,1006] modularize with comments
 *  * [6924,1036] add TIB buffer handler
 *
 *> 2020-08 circuit4u@medium.com (Tiny Forth NXT), Arduino reference<br>
 *
 *> 2004-07 T. NAKAGAWA (Tiny FORTH), original reference<br>
 */

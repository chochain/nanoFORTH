/**
 * @file nanoforth.cpp
 * @brief nanoForth main controller class implementation
 *
 * Refactor History:
 * -----------------
 *> 2004-07 T. NAKAGAWA (Tiny FORTH), original reference<br>
 *> 2020-08 circuit4u@medium.com (Tiny Forth NXT), Arduino reference<br>
 *> 2021-03 chochain@yahoo.com (CC Lee)
 *  * [6698,1006] modularize with comments
 *  * [6924,1036] add TIB buffer handler
 *
 *> 2021-04-02: chochain@yahoo.com
 *  * [6782,1056] add protothread multi-tasker
 *  * [6930,1056] add threads
 *  * [6908,1014] use F()
 *  * [6860,852]  use PROGMEM, pgm_read_byte in find()
 *  * [6906,852]  getnum support negative number
 *  * [7012,860]  add digitalR/W
 *
 *> 2021-04-03: chochain@yahoo.com
 *  * [6898,852]  add vm_delay
 *  * [6912,852]  update EEPROM IO
 *  * [6842,852]  execution tracing enable/disable
 *  * [6922,852]  add CELL (CEL), ALLOT (ALO) opcodes
 *
 *> 2021-04-16: chochain@yahoo.com
 *  * [7446,932]  add list_word, use 80-byte console buffer
 *  * [7396,932]  add ASM_TRACE from EXE_TRACE options
 *  * [7676,802]  grow Task at end of heap
 *  * [8266,802]  if use array instead of pointer arithmetics, revert!
 *
 *> 2021-05-03: chochain@yahoo.com
 *  * [7818,1428] forget Task struct; grow MEM_SZ to DIC_SZ+STK_SZ (1K+64*2)
 *  * [8254,1430] add execution tracing option
 *
 *> 2021-05-06: chochain@yahoo.com
 *  * [9214,1424] refactor to C++ for Arduino lib
 *  * [11750,280] add mem,stk sizing; VM and ASM objects dynamically created
 *  * [11956,272] add struct n4_task and N4_FUNC multi-tasker
 *
 *> 2021-05-11: chochain@yahoo.com
 *  * [12124,270] reformat opcodes for 64-primitives
 *  * [12420,270] display more system memory info and malloc error catch
 *
 *> 2021-05-16: chochain@yahoo.com - v1.2
 *  * [12360,272] move static variables into NanoForth class
 *  * [15180,472] add EEPROM; BlueTooth/BLE<=>stream IO
 *
 *> 2022-01-20: chochain@yahoo.com - v1.4
 *  * [15634,472] autorun; handles Forth comments
 */
#include "nanoforth_vm.h"
//
// user function linked-list
//
n4_tptr NanoForth::_n4tsk{ NULL };                       ///< initialize task linked-list (static member)
///
/// @return
///   0 - if all allocation are OK<br>
///   1 - any allocation failure
///
int NanoForth::begin(Stream &io, U8 ucase, U16 mem_sz, U16 stk_sz)
{
    _mem  = (U8*)malloc(mem_sz);                         /// * allocate Forth memory block
    _n4vm = new N4VM(io, ucase, _mem, mem_sz, stk_sz);   /// * create Virtual Machine
    
    if (!_mem || !_n4vm) return -1;
    
#if ARDUINO
    _n4vm->meminfo();                                    // display detailed pointers
#else     
    log("MEM=$");   logx(mem_sz);                        // forth memory block
    log("[DIC=$");  logx(mem_sz - stk_sz);               // dictionary size
    log(",STK=$");  logx(stk_sz);                        // stack size
    log("]\n");
#endif // ARDUINO

    return 0;
}
///
///> add new (user defined) hardware task to linked-list
///
void NanoForth::add(void (*ufunc)(n4_tptr))
{
    n4_tptr tp = (n4_tptr)malloc(sizeof(n4_task));

    tp->func = ufunc;   /// * assign user function
    tp->ci   = 0;       /// * reset case index 
    tp->next = _n4tsk;  /// * push into linked-list
    _n4tsk   = tp;      /// * reset head
}
///
///> n4 execute one line of command from input buffer
///
void NanoForth::exec()
{
    while (_n4vm->step()) {                    /// * step through commands from input buffer
        yield();
    }
}
///
///> n4 yield, execute one round of user hardware tasks
///
void NanoForth::yield()
{
    for (n4_tptr tp=_n4tsk; tp; tp=tp->next) { /// * follow task linked list
        tp->func(tp);                          /// * execute task function once
    }
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
#if !ARDUINO
int main(int argc, char **argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);       // autoflush (turn STDOUT buffering off)
    
    NanoForth n4;
    n4.begin();
    while (1) {
        n4.exec();
    }
    return 0;
}
#endif // !ARDUINO



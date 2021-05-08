/*
 nanoFORTH - Forth for Arduino Nano (and UNO)

 2004-07 T. NAKAGAWA (Tiny FORTH), original reference 
 2020-08 circuit4u@medium.com (Tiny Forth NXT), Arduino reference
 2021-03 CC Lee: modularize with comments [6698,1006]; add buffer handler[6924,1036]
 2021-04-02: CC - add multi-tasker, byte count[6782/1056],
    [6930,1056] add threads
    [6908,1014] use F()
    [6860,852]  use PROGMEM, pgm_read_byte in find()
    [6906,852]  getnum support negative number
    [7012,860]  add digitalR/W
  2021-04-03: CC
    [6898,852]  add vm_delay
    [6912,852]  update EEPROM IO
    [6842,852]  execution tracing enable/disable
    [6922,852]  add CELL (CEL), ALLOT (ALO) opcodes
  2021-04-16: CC
    [7446,932]  add list_word, use 80-byte console buffer
    [7396,932]  add ASM_TRACE from EXE_TRACE options
    [7676,802]  grow Task at end of heap
    [8266,802]  if use array instead of pointer arithmetics, revert!
  2021-05-03: CC
    [7818,1428] forget Task struct; grow MEM_SZ to DIC_SZ+STK_SZ (1K+64*2)
    [8254,1430] add execution tracing option
  2021-05-06: CC
    [9214,1424] refactor to C++ for Arduino lib
*/
#include <pt.h>
#include "nanoforth_vm.h"

#define n4_delay( ms)  do {                 \
    static U32 t;                           \
    t = millis() + (U32)(ms);               \
    PT_WAIT_UNTIL(&_n4hw_ctx, millis()>=t); \
} while(0)
//
// main hardward protothread
//
struct pt _n4hw_ctx;                        // hardware context
PT_THREAD(_n4hw_thread())                   // hardward protothread
{
    PT_BEGIN(&_n4hw_ctx);
    /*
    U16 tmp;
    find("LD ", LST_EXT, &tmp);             // Load DIC
    extended((U8)tmp);
    
    if (lookup("INI", &tmp)) {              // RUN "INI"
        execute(tmp + 2 + 3);               // header: 2-byte pointer to next + 3-byte NAME
    }  
    */
    while (1) {
//        n4_loop();                          // calling user function
        digitalWrite(LED_BUILTIN, HIGH);
        n4_delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        n4_delay(500);
    }
    PT_END(&_n4hw_ctx);
}
/*
int n4_delay(U32 ms)
{
    PT_BEGIN(&_n4hw_ctx);
    U32 t = millis() + ms;
    PT_WAIT_UNTIL(&_n4hw_ctx, millis()>=t);
    PT_END(&_n4hw_ctx);
}
*/

NanoForth::NanoForth() : NanoForth(MEM_SZ, STK_SZ) {}
NanoForth::NanoForth(U16 mem_sz, U16 stk_sz)
{
    U8 *_mem = (U8*)malloc(mem_sz);    // allocate heap
    N4VM v0(_mem, mem_sz, stk_sz);     // instanciate NanoForth VM
    
    vm = &v0;                          // Arduino has no new()
    vm->info();
    
    PT_INIT(&_n4hw_ctx);
}
//
// single step for Arduino loop
//
bool NanoForth::run()
{
    vm->step();
//    yield();
}
//
// n4 yield to hardware context
//
void NanoForth::yield()
{
    PT_SCHEDULE(_n4hw_thread());           // context switch to hardware
}
//
// console input with cooperative threading
//
char NanoForth::key()
{
    while (!Serial.available()); // yield();
    
    return Serial.read();
}
//
// aka Arduino delay(), yield to hardware context while waiting
//
void NanoForth::wait(U32 ms)
{
    U32 t = millis() + ms;
    while (millis()<t) yield();
}
//
// for Eclipse debugging
//
/*
int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);		// autoflush (turn STDOUT buffering off)
    
	NanoForth n4;
	n4.run();
	return 0;
}
*/



#include <pt.h>
#include "nanoforth_vm.h"
//
// thread handler (note: using macro to make it stackless)
//
static struct pt _n4hw_ctx;                 // protothread contexts
static void (*_n4hw_f)();                   // function pointer
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
        _n4hw_f();
    }
    PT_END(&_n4hw_ctx);
}
//
// NanoForth multi-tasking handler
//
#define HW_DELAY(ms)  do {                  \
} while(0)

NanoForth::NanoForth()
{
    static U8 _mem[MEM_SZ];       // allocate heap

    vm = new N4VM;
    vm->init(_mem, MEM_SZ, STK_SZ);
    
    PT_INIT(&_n4hw_ctx);          // initialize hardware thread
}
//
// assign user function pointer
//
void NanoForth::set_function(void (*f)())
{
    _n4hw_f = f;
}
//
// single step for Arduino loop
//
void NanoForth::step()
{
    vm->step();
    n4_yield();                                // share some CPU cycle to user routine
}
//
// static members
//
void NanoForth::n4_yield()
{
    PT_SCHEDULE(_n4hw_thread());               // give hardware some CPU time
}
//
// console input with cooperative threading
//
char NanoForth::n4_getchar()
{
    while (!Serial.available()) n4_yield();
    
    return Serial.read();
}

void NanoForth::n4_delay(U32 ms)
{
    U32 t = millis() + ms;
    while (millis()<t) n4_yield();
}

void NanoForth::hw_delay(U32 ms)
{
    static U32 t;
    t = millis() + (U32)(ms);
    //PT_WAIT_UNTIL(&_n4hw_ctx, millis()>=t);
}


#include <pt.h>
#include "nanoforth_vm.h"

struct pt _n4hw_ctx;                        // hardware protothread context
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
        n4_loop();
    }
    PT_END(&_n4hw_ctx);
}

int n4_delay(U32 ms)
{
    PT_BEGIN(&_n4hw_ctx);
    U32 t = millis() + ms;
    PT_WAIT_WHILE(&_n4hw_ctx, millis()<t);
    PT_END(&_n4hw_ctx);
}

NanoForth::NanoForth()
{
    static U8 _mem[MEM_SZ];       // allocate heap

    vm = new N4VM;
    vm->init(_mem, MEM_SZ, STK_SZ);

    PT_INIT(&_n4hw_ctx);
}
//
// single step for Arduino loop
//
bool NanoForth::run()
{
    vm->step();
    yield();
}
//
// console input with cooperative threading
//
char NanoForth::key()
{
    while (!Serial.available()) yield();
    
    return Serial.read();
}

void NanoForth::yield()
{
    PT_SCHEDULE(_n4hw_thread());           // context switch to hardware
}

void NanoForth::wait(U32 ms)
{
    U32 t = millis() + ms;
    while (millis()<t) yield();
}





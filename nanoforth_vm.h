#ifndef __SRC_NANOFORTH_VM_H
#define __SRC_NANOFORTH_VM_H
#include "nanoforth.h"

class N4Asm;
class N4VM
{
    N4Asm  *n4asm;      // assembler 
    U16    msz;         // memory size        mem[dic->...<-stk]
    U16    ssz;         // stack size
    U8     trc;         // tracing flags
    
    U8     *dic;        // dictionary base
    U16    *rp;         // return stack pointer
    S16    *sp;         // parameter stack pinter
    
public:
    N4VM(U8 *mem, U16 mem_sz, U16 stk_sz);
    
    void step();
    void info();
    void set_trace(U16 f);
    
private:
    void _init();
    void _ok();              // console prompt (with stack dump)
    //
    // VM execution units
    //
    void _execute(U16 adr);  // opcode execution unit
    void _primitive(U8 op);  // execute a primitive instruction
    void _extended(U8 op);   // extended opcode dispatcher
};
#endif //__SRC_NANOFORTH_VM_H

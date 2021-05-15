///
/// \file nanoforth_vm.h
/// \brief NanoForth Virtual Machine class
///
#ifndef __SRC_NANOFORTH_VM_H
#define __SRC_NANOFORTH_VM_H
#include "nanoforth.h"

class N4Asm;            // forward declaration
///
/// NanoForth Virtual Machine class
///
class N4VM
{                       //  (16-byte header)
    N4Asm  *n4asm;      ///< assembler object pointer
    
    U16    msz;         ///< memory size        mem[dic->...<-stk]
    U16    ssz;         ///< stack size
    
    U8     *dic;        ///< dictionary base
    U16    *rp;         ///< return stack pointer
    S16    *sp;         ///< parameter stack pinter
    
    U8     trc;         ///< tracing flags
    U8     xxx;         ///< reserved
    
public:
    N4VM();                   ///< NanoForth Virtual Machine constructor
    /// intializer (Arduino does not have dynamic constructor)
    int init(
        U8 *mem,              ///< pointer to a heap block
        U16 mem_sz,           ///< memory size
        U16 stk_sz            ///< parameter/return stack size
        );
    
    void info();              ///< display VM system info
    void step();              ///< execute one-cycle of virtual machine
    void set_trace(U16 f);    ///< enable/disable execution tracing
    
private:
    void _init();             ///< restart virtual machine (reseting internals)
    void _ok();               ///< console prompt (with stack dump)

    /// VM execution units
    void _execute(U16 adr);   ///< opcode execution unit
    void _primitive(U8 op);   ///< execute a primitive instruction
};
#endif //__SRC_NANOFORTH_VM_H

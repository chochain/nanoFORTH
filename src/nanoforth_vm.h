/**
 * @file nanoforth_vm.h
 * @brief nanoForth Virtual Machine class
 *
 */
#ifndef __SRC_NANOFORTH_VM_H
#define __SRC_NANOFORTH_VM_H
#include "nanoforth_core.h"

class N4Asm;                   // forward declaration
///
/// nanoForth Virtual Machine class
///
class N4VM : N4Core
{                             //  (12-byte header)
    N4Asm  *n4asm;            ///< assembler object pointer
    U16    dsz;               ///< dictionary size

public:
    // nanoForth Virtual Machine constructor
    N4VM(
        Stream &io,           ///< IO stream
        U8 ucase,             ///< case sensitiveness
        U8 *dic,              ///< dictionary base pointer
        U16 dic_sz,           ///< dictionary block size
        U16 stk_sz            ///< stack block size
        );

    void meminfo();           ///< display VM system info
    U8   step();              ///< execute one-cycle of virtual machine

private:
    void _init();             ///< restart virtual machine (reseting internals)
    void _ok();               ///< console prompt (with stack dump)

    ///@name VM execution units
    ///@{
    void _nest(U16 adr);      ///< execute a colon word
    void _invoke(U8 op);      ///< invoke a built-in opcode
    ///@}
    ///@name Memory Dumper
    ///@{
    void _dump(               ///< mem block with dictionary offset and length
        U16 p0,               ///< starting dictionary address
        U16 sz0               ///< number of bytes to print
        );
    ///@}
};
#endif //__SRC_NANOFORTH_VM_H

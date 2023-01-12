/**
 * @file nanoforth_vm.h
 * @brief nanoForth Virtual Machine class
 *
 */
#ifndef __SRC_NANOFORTH_VM_H
#define __SRC_NANOFORTH_VM_H
#include "nanoforth.h"
///
/// nanoForth Virtual Machine class
///
namespace N4VM
{                             //  (12-byte header)
    // nanoForth Virtual Machine constructor
    void setup(
        Stream &io,           ///< IO stream
        U8 ucase,             ///< case sensitiveness
        U8 *dic,              ///< dictionary base pointer
        U16 dic_sz,           ///< dictionary block size
        U16 stk_sz            ///< stack block size
        );
    void meminfo();           ///< display VM system info
    void outer();             ///< outer-interpreter
    void isr();               ///< interrupt service routine
};  // namespace N4VM
#endif //__SRC_NANOFORTH_VM_H

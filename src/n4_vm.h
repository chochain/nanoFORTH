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
{
    // nanoForth Virtual Machine constructor
    void setup(
        Stream &io,           ///< IO stream
        U8 ucase              ///< case sensitiveness
        );
    void outer();             ///< outer-interpreter
    void serv_isr();          ///< interrupt service routine
};  // namespace N4VM
#endif //__SRC_NANOFORTH_VM_H

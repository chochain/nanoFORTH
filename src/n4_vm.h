/**
 * @file
 * @brief nanoForth Virtual Machine class
 *
 */
#ifndef __SRC_N4_VM_H
#define __SRC_N4_VM_H
#include "n4.h"
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
#endif //__SRC_N4_VM_H

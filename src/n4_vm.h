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
    	const char *code,     ///< preload Forth code
        U8 ucase,             ///< case sensitiveness
        Stream &io            ///< IO stream
        );
    void outer();             ///< outer-interpreter
    void serv_isr();          ///< interrupt service routine
};  // namespace N4VM
#endif //__SRC_N4_VM_H

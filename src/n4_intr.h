/**
 * @file
 * @brief nanoForth interrupt handler class
 */
#ifndef __SRC_N4_INTR_H
#define __SRC_N4_INTR_H
#include "n4_core.h"

#if ARDUINO
#define CLI()   cli()
#define SEI()   sei()
#else  // !ARDUINO
#define LOW     0
#define HIGH    1
#define CLI()
#define SEI()
#endif // ARDUINO

namespace N4Intr {
    void reset();                  ///< reset interrupts
    U16  isr();                    ///< fetch interrupt service routines

    void add_tmisr(
            U16 i,                 ///< interrupt handler slot#
            U16 n,                 ///< interrupt period (n x 10ms = period)
            U16 xt);               ///< handler's xt
    void add_pcisr(
            U16 pin,               ///< pin change to capture
            U16 xt);               ///< handler's xt

    void enable_timer(U16 f);      ///< ENABLE=1, DISABLE=0
    void enable_pci(U16 f);
};    // namespace N4Intr

#endif //__SRC_N4_INTR_H

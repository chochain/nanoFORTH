/**
 * @file
 * @brief nanoForth interrupt handler class
 */
#ifndef __SRC_N4_INTR_H
#define __SRC_N4_INTR_H
#include "n4_core.h"

#define ISR_PERIOD 100             /* tick divider, higher => longer */

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
    extern U16  p_xt[3];           ///< pin change ISR
    extern U16  t_xt[8];           ///< timer ISR

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

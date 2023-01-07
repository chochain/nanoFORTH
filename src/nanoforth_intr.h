/**
 * @file
 * @brief nanoForth interrupt handler class
 */
#ifndef __SRC_NANOFORTH_INTR_H
#define __SRC_NANOFORTH_INTR_H
#include "nanoforth_core.h"

#if ARDUINO
#define CLI()   cli()
#define SEI()   sei()
#else  // !ARDUINO
#define pinMode(p,v)
#define digitalWrite(p,v)
#define digitalRead(p)              1
#define LOW                         0
#define HIGH                        1
#define CLI()
#define SEI()
#endif // ARDUINO

namespace N4Intr {
	extern U8   t_idx;             ///< timer ISR index
	extern U16  p_xt[3];           ///< pin change ISR
	extern U16  t_xt[8];           ///< timer ISR

    void reset();                  ///< reset interrupts
    U16  hits();                   ///< fetch interrupt service routines

    void add_timer(U16 prd, U16 xt);
    void add_pci(U16 pin, U16 xt);
    void enable_timer(U16 f);
    void enable_pci(U16 f);
};    // namespace N4Intr

#endif //__SRC_NANOFORTH_INTR_H

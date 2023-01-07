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

struct N4Intr {
    volatile static U8  t_hit;     ///< 8-bit for 8 timer ISR vectors
    volatile static U8  p_hit;     ///< pin change interrupt
    volatile static U16 t_cnt[8];  ///< timer counter

    static U8   t_idx;             ///< timer ISR index
    static U8   xxx;               ///< reserved
    static U16  p_xt[3];           ///< pin change ISR
    static U16  t_max[8];          ///< timer CTC value
    static U16  t_xt[8];           ///< timer ISR

    static void reset();           ///< reset interrupts
    static U16  hits();            ///< fetch interrupt service routines

    static void add_timer(U16 prd, U16 xt);
#if ARDUINO
    static void add_pci(U16 pin, U16 xt);
    static void enable_timer(U16 f);
    static void enable_pci(U16 f);
#else // !ARDUINO (mocked interrupt handlers)
    static void add_pci(U16 pin, U16 xt) {}
    static void enable_timer(U16 f)      {}
    static void enable_pci(U16 f)           {}
#endif
};
#endif //__SRC_NANOFORTH_INTR_H

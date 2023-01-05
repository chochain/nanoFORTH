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
#define CLI()
#define SEI()
#endif // ARDUINO

typedef void (N4VM::* FPTR)(U16);
struct N4Intr {
	static U8   p_hit;           ///< pin change interrupt
	static U8   t_hit;           ///< 8-bit for 8 vectors
	static U8   t_idx;           ///< timer ISR index
	static U8   xxx;             ///< reserved

    static U16  p_xt[3];         ///< pin change ISR
    static U16  t_cnt[8];        ///< timer counter
    static U16  t_max[8];        ///< timer CTC value
    static U16  t_xt[8];         ///< timer ISR

    static void reset();         ///< reset interrupts
    static U16  hit(U16 *xt);    ///< fetch interrupt service routines

    static void add_timer(U16 prd, U16 xt);
#if ARDUINO
    static void add_pci(U16 pin, U16 xt);
    static void enable_timer(U16 f);
    static void enable_pci(U16 f);
#else // !ARDUINO (mocked interrupt handlers)
    static void add_pci(U16 pin, U16 xt) {}
    static void enable_timer(U16 f)      {}
    static void enable_pci(U16 f)  		 {}
#endif
};
#endif //__SRC_NANOFORTH_INTR_H

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

	static U8   p_hit;           ///< pin change interrupt
	static U8   t_hit;           ///< 8-bit for 8 vectors
	static U8   t_idx;           ///< timer ISR index
	static U8   xxx;             ///< reserved

    static U16  p_xt[3];         ///< pin change ISR
    static U16  t_cnt[8];        ///< timer counter
    static U16  t_max[8];        ///< timer CTC value
    static U16  t_xt[8];         ///< timer ISR

void intr_reset();
void intr_service(FPTR fn);

    static void intr_add_timer(U16 prd, U16 adr);
#if ARDUINO
    static void add_pci(U16 pin, U16 adr);
    static void enable_timer(U16 f);
    static void enable_pci(U16 f);
#else // !ARDUINO (mocked interrupt handlers)
    static void intr_add_pci(U16 pin, U16 adr)   {}
    static void enable_timer(U16 f)         {}
    static void enable_pci(U16 f)  			{}
#endif
};
#endif //__SRC_NANOFORTH_INTR_H

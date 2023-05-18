/**
 * @file
 * @brief nanoForth Interrupt handlers implementation
 *    Note: with volatile struct reduce 100 cycles from 14ms to 11ms
 */
#include "n4_intr.h"
///
/// nanoForth Interrupt handler -  static variables
///
typedef struct {
    U8  t_idx { 0 };           ///< max timer interrupt slot index
    U16 t_max[8];              ///< timer CTC top value
    U16 xt[11];                ///< vectors 0-7: timer, 8-10 pin change
    volatile U16 t_cnt[8];     ///< timer CTC counters
    volatile U8  t_hit { 0 };  ///< 8-bit for 8 timer ISR,
    volatile U8  p_hit { 0 };  ///< 3-bit for pin change ISR
} IsrRec;                      ///< Interrupt Record Keeper

namespace N4Intr {
IsrRec ir;                     ///< real-time interrupt record

void reset() {
    CLI();
    ir.t_idx = ir.t_hit = ir.p_hit = 0;
    for (U8 i=0; i < 11; i++) ir.xt[i] = 0;
    SEI();
}
#if ARDUINO
#define _fake_intr(hx)
#else // !ARDUINO
U8  tmr_on = 0;               ///< fake timer enabler
void _fake_intr(U16 hx)
{
    static U8 n = 0;               // fake interrupt
    if (tmr_on && !hx && ++n >= 50) {
        n=0; ir.t_hit = 3;
    }
}
#endif // ARDUINO
///
///> fetch interrupt service routine if any
///
#define ISR_THROTTLE 100           /** interrupt throttle count */
U16 isr() {
    volatile static U8  hit = 0;   ///> 8-bit flag makes checking faster
    volatile static U16 hx  = 0;   ///> cached interrupt flags
    static U8 cnt = 0;             ///> interrupt throttle counter (256 max)

    _fake_intr(hx);

    if (!hit && ++cnt < ISR_THROTTLE) return 0;
    cnt = 0;

    CLI();
    if (!hit) {                    // collect interrupts if no existing one to serve
        hit = (hx = ((U16)ir.p_hit << 8) | ir.t_hit) != 0;
        ir.p_hit = ir.t_hit = 0;
    }
    SEI();
    for (U8 i=0; hx; i++, hx>>=1) {// serve interrupts (hopefully fairly)
        if (hx & 1) {              // check interrupt flag
            hx >>= 1;              // clear flag
            return ir.xt[i];       // return ISR to Forth VM
        }
    }
    return hit = 0;
}
void add_tmisr(U16 i, U16 n, U16 xt) {
    if (xt==0 || i > 7) return;    // range check

    CLI();
    ir.xt[i]    = xt;                      // ISR xt
    ir.t_cnt[i] = (U16)millis() % n;       // init counter (randomize, spread time slice)
    ir.t_max[i] = n;                       // period (in 1ms)
    if (i >= ir.t_idx) ir.t_idx = i + 1;   // cache max index
    SEI();
}
#if !ARDUINO
void add_pcisr(U16 p, U16 xt) {}   // mocked functions for x86
void enable_pci(U16 f)        {}
void enable_timer(U16 f)      { tmr_on = f; }
#else  // ARDUINO
///
///@name N4Intr static variables
///@{
void add_pcisr(U16 p, U16 xt) {
    if (xt==0) return;               // range check
    CLI();
    if (p < 8)       {
        ir.xt[10] = xt;
        PCMSK2 |= 1 << p;
    }
    else if (p < 13) {
        ir.xt[8] = xt;
        PCMSK0 |= 1 << (p - 8);
    }
    else {
        ir.xt[9] = xt;
        PCMSK1 |= 1 << (p - 14);
    }
    SEI();
}
void enable_pci(U16 f) {
    CLI();
    if (f) {
        if (ir.xt[8])  PCICR |= _BV(PCIE0);     // enable PORTB
        if (ir.xt[9])  PCICR |= _BV(PCIE1);     // enable PORTC
        if (ir.xt[10]) PCICR |= _BV(PCIE2);     // enable PORTD
    }
    else PCICR = 0;
    SEI();
}
void enable_timer(U16 f) {
    CLI();
    TCCR2A = TCCR2B = TCNT2 = 0;                // reset counter
    if (f) {
        TCCR2A = _BV(WGM21);                    // Set CTC mode
        TCCR2B = _BV(CS22);                     // prescaler 64 (16MHz / 64) = 250KHz => 4us period
        OCR2A  = 249;                           // 250x4us = 1ms, (250 - 1, must < 256)
        TIMSK2 |= _BV(OCIE2A);                  // enable timer2 compare interrupt
    }
    else {
        TIMSK2 &= _BV(OCIE2A);                  // disable timer2 compare interrupt
    }
    SEI();
}
#endif // ARDUINO

};  // namespace N4Intr
///
/// Arduino interrupt service routines (1ms precision)
///
#if ARDUINO
ISR(TIMER2_COMPA_vect) {
    for (U8 i=0, b=1; i < N4Intr::ir.t_idx; i++, b<<=1) {
        if (!N4Intr::ir.xt[i] ||               // check against stop counters
            (++N4Intr::ir.t_cnt[i] < N4Intr::ir.t_max[i])) continue;
        N4Intr::ir.t_hit    |= b;              // mark hit bit
        N4Intr::ir.t_cnt[i]  = 0;              // reset counter
    }
}
ISR(PCINT0_vect) { N4Intr::ir.p_hit |= 1; }    // mark hit bit
ISR(PCINT1_vect) { N4Intr::ir.p_hit |= 2; }
ISR(PCINT2_vect) { N4Intr::ir.p_hit |= 4; }
#endif // ARDUINO

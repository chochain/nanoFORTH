/**
 * @file
 * @brief nanoForth Interrupt handlers implementation
 */
#include "n4_intr.h"
///
/// nanoForth Interrupt handler -  static variables
///
namespace N4Intr {

U8  t_idx   = { 0 };       ///< max timer interrupt slot index
U16 p_xt[3] = { 0, 0, 0 }; ///< pin change interrupt vectors
U16 t_xt[8];               ///< timer interrupt vectors
U16 t_max[8];              ///< timer CTC top value
    
volatile U8  _hits { 0 };  ///< interrupt flags
volatile U8  p_hit { 0 };  ///< pin change interrupt (PORT-B,C,D)
volatile U8  t_hit { 0 };  ///< 8-bit for 8 timer ISR
volatile U16 t_cnt[8];     ///< timer CTC counters

void reset() {
    CLI();
    t_idx = t_hit = p_hit = 0;
    for (int i=0; i < 8; i++) t_xt[i] = 0;
    for (int i=0; i < 3; i++) p_xt[i] = 0;
    SEI();
}
#if ARDUINO
#define _fake_intr()
#else // !ARDUINO
U8  tmr_on = 0;                   ///< fake timer enabler
void _fake_intr()
{
    static int n = 0;                              // fake interrupt
    if (tmr_on && !_hits && ++n >= 2) {
        n=0; t_hit = 1;
    }
}
#endif // ARDUINO
///
///> fetch interrupt service routine if any
///
U16 isr() {
    static U16 n = 0;

    _fake_intr();
    
    if (!_hits && ++n < ISR_PERIOD) return 0;
    n = 0;
    CLI();
    if (!_hits) {
        _hits = (p_hit << 8) | t_hit; // capture interrupt flags
        t_hit = p_hit = 0;            // clear flags, ready for next round
    }
    SEI();
    if (_hits) {
        U8 hx = _hits & 0xff;
        for (int i=0, t=1; hx && i < t_idx; i++, t<<=1, hx>>=1) {
            if (_hits & t) { _hits &= ~t; return t_xt[i]; }
        }
        hx = _hits >> 8;
        for (int i=0, t=0x100; hx && i < 3; i++, t<<=1, hx>>=1) {
            if (_hits & t) { _hits &= ~t; return p_xt[i]; }
        }
    }
    return 0;
}
void add_tmisr(U16 i, U16 n, U16 xt) {
    if (xt==0 || i > 7) return;      // range check

    CLI();
    t_xt[i]  = xt;                   // ISR xt
    t_cnt[i] = 0;                    // init counter
    t_max[i] = n;                    // period (in 1ms)
    if (i >= t_idx) t_idx = i + 1;   // cache max index
    SEI();
}
#if !ARDUINO
void add_pcisr(U16 p, U16 xt) {}     // mocked functions for x86
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
        p_xt[2] = xt;
        PCMSK2 |= 1 << p;
    }
    else if (p < 13) {
        p_xt[0] = xt;
        PCMSK0 |= 1 << (p - 8);
    }
    else {
        p_xt[1] = xt;
        PCMSK1 |= 1 << (p - 14);
    }
    SEI();
}
void enable_pci(U16 f) {
    CLI();
    if (f) {
        if (p_xt[0]) PCICR |= _BV(PCIE0);  // enable PORTB
        if (p_xt[1]) PCICR |= _BV(PCIE1);  // enable PORTC
        if (p_xt[2]) PCICR |= _BV(PCIE2);  // enable PORTD
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
    for (U8 i=0, b=1; i < N4Intr::t_idx; i++, b<<=1) {
        if (!N4Intr::t_xt[i] ||
            (++N4Intr::t_cnt[i] < N4Intr::t_max[i])) continue;
        N4Intr::t_hit    |= b;
        N4Intr::t_cnt[i]  = 0;
    }
}
ISR(PCINT0_vect) { N4Intr::p_hit |= 1; }
ISR(PCINT1_vect) { N4Intr::p_hit |= 2; }
ISR(PCINT2_vect) { N4Intr::p_hit |= 4; }
#endif // ARDUINO

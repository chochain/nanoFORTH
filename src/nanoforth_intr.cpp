/**
 * @file
 * @brief nanoForth Interrupt handlers implementation
 */
#include "nanoforth_intr.h"
///
/// Interrupt handler -  static variables
///
U8  N4Intr::t_idx   { 0 };
U16 N4Intr::p_xt[]  { 0, 0, 0 };
U16 N4Intr::t_xt[]  { 0, 0, 0, 0, 0, 0, 0, 0 };
U16 N4Intr::t_max[] { 0, 0, 0, 0, 0, 0, 0, 0 };
volatile U8  N4Intr::t_hit   { 0 };
volatile U8  N4Intr::p_hit   { 0 };
volatile U16 N4Intr::t_cnt[] { 0, 0, 0, 0, 0, 0, 0, 0 };

static U8 LED[] = { 7, 4, 5, 6, 16, 17, 18 };

void N4Intr::reset() {
    CLI();
    t_idx = t_hit = p_hit = 0;
    SEI();
    for (int i=0; i<sizeof(LED)/sizeof(U8); i++) {
        pinMode(LED[i], OUTPUT);
    }
}
///
///> fetch interrupt hit flags
/// Note: once flags are collected, hit flags are clear
///
U16 N4Intr::hits(U16 *xt) {
//    if (!(t_hit || p_hit)) return 0;   // quick exit
    U16 n = 0;
    CLI();
    for (int i=0; t_hit && i<t_idx; i++, t_hit>>=1) {
        if (t_hit & 1) xt[n++] = t_xt[i];
    }
    for (int i=0; p_hit && i<3; i++, p_hit>>=1) {
        if (p_hit & 1) xt[n++] = p_xt[i];
    }
    SEI();
    digitalWrite(16+n, digitalRead(16+n) ? LOW : HIGH);
    return n;
}
void N4Intr::add_timer(U16 n, U16 xt)
{
    if (xt==0 || t_idx > 7) return;  // range check

    t_xt[t_idx]  = xt;               // ISR xt
    t_cnt[t_idx] = 0;                // init counter
    t_max[t_idx] = n;                // period (in 0.1s)
    
    CLI();
    t_idx++;
    SEI();
}
#if ARDUINO
///
///@name N4Intr static variables
///@{
void N4Intr::add_pci(U16 p, U16 xt) {
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

void N4Intr::enable_pci(U16 f) {
    CLI();
    if (f) {
        if (p_xt[0]) PCICR |= _BV(PCIE0);  // enable PORTB
        if (p_xt[1]) PCICR |= _BV(PCIE1);  // enable PORTC
        if (p_xt[2]) PCICR |= _BV(PCIE2);  // enable PORTD
    }
    else PCICR = 0;
    SEI();
}
void N4Intr::enable_timer(U16 f) {
    CLI();
    pinMode(7, OUTPUT);
    digitalWrite(7, HIGH);

    TCCR2A = TCCR2B = TCNT2 = 0;                // reset counter
    if (f) {
        TCCR2A = _BV(WGM21);                    // Set CTC mode
        TCCR2B = _BV(CS22)|_BV(CS21);           // prescaler 256 (16000000 / 256) = 62500Hz = 16us
        OCR2A  = 249;                           // 250Hz = 4ms, (250 - 1, must < 256)
        TIMSK2 |= _BV(OCIE2A);                  // enable timer2 compare interrupt
    }
    else {
        TIMSK2 &= _BV(OCIE2A);                 // disable timer2 compare interrupt
        digitalWrite(4, LOW);
        digitalWrite(5, LOW);
        digitalWrite(6, LOW);
    }
    SEI();
}
///
/// Arduino interrupt service routines
///
ISR(TIMER2_COMPA_vect) {
    volatile static int cnt = 0;
    if (++cnt < 25) return;                     // 25 * 4ms = 100ms
    cnt = 0;
    for (U8 i=0, b=1; i < N4Intr::t_idx; i++, b<<=1) {
        digitalWrite(7, digitalRead(7) ? LOW : HIGH);
        if (++N4Intr::t_cnt[i] < N4Intr::t_max[i]) continue;
        N4Intr::t_hit    |= b;
        N4Intr::t_cnt[i]  = 0;
        digitalWrite(4+N4Intr::t_hit, digitalRead(4+N4Intr::t_hit) ? LOW : HIGH);
    }
}
ISR(PCINT0_vect) { N4Intr::p_hit |= 1; }
ISR(PCINT1_vect) { N4Intr::p_hit |= 2; }
ISR(PCINT2_vect) { N4Intr::p_hit |= 4; }
#endif // ARDUINO

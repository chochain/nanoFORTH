/**
 * @file
 * @brief nanoForth Interrupt handlers implementation
 */
#include "nanoforth_intr.h"
///
/// Interrupt handler -  static variables
///
U8  N4Intr::t_idx   { 0 };
U8  N4Intr::t_hit   { 0 };
U8  N4Intr::p_hit   { 0 };
U16 N4Intr::p_xt[]  { 0, 0, 0 };
U16 N4Intr::t_xt[]  { 0, 0, 0, 0, 0, 0, 0, 0 };
U16 N4Intr::t_cnt[] { 0, 0, 0, 0, 0, 0, 0, 0 };
U16 N4Intr::t_max[] { 0, 0, 0, 0, 0, 0, 0, 0 };

void N4Intr::reset() {
    CLI();
    p_hit = t_hit = t_idx = 0;
    SEI();
}
void N4Intr::add_timer(U16 n, U16 adr)
{
    if (t_idx > 7) return;                  // range check

    t_xt[t_idx]  = adr + 2 + 3;             // ISR word (adr + lnk[2] + name[3])
    t_cnt[t_idx] = 0;                       // init counter
    t_max[t_idx] = n;                       // period (in 0.1s)

    t_idx++;
}
void N4Intr::service(FPTR fn) {
	if (!(t_hit || p_hit)) return;
    //
	//
	//
    CLI();
    t_hit = p_hit = 0;
    SEI();
}
#if ARDUINO
///
///@name N4Intr static variables
///@{
void N4Intr::add_pci(U16 p, U16 xt) {
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

void N4Intr::enable_pci() {
    CLI();
	if (p_xt[0]) PCICR |= _BV(PCIE0);  // enable PORTB
	if (p_xt[1]) PCICR |= _BV(PCIE1);  // enable PORTC
    if (p_xt[2]) PCICR |= _BV(PCIE2);  // enable PORTD
    SEI();
}
void N4Intr::disable_pci() {
    CLI();
    PCICR = 0;
    SEI();
}
void N4Intr::enable_timer() {
    CLI();
    
    TCCR2A = TCCR2B = TCNT2 = 0;            // reset counter
    TCCR2A = _BV(WGM21);                    // Set CTC mode
    TCCR2B = _BV(CS22)|_BV(CS21);           // prescaler 256 (16000000 / 256) = 62500Hz = 16us
    OCR2A  = 249;                           // 250Hz = 4ms, (250 - 1, must < 256)
    TIMSK2 |= _BV(OCIE2A);                  // enable timer2 compare interrupt
    
    SEI();
}

void N4Intr::disable_timer() {
    CLI();
    TIMSK2 &= _BV(OCIE2A);                  // enable timer2 compare interrupt
    SEI();
}
///
/// Arduino interrupt service routines
///
ISR(TIMER2_COMPA_vect) {
    volatile static int cnt = 0;
    if (++cnt < 25) return;                 // 25 * 4ms = 100ms
    cnt = 0;
    for (int i=0; i < N4Intr::t_idx; i++) {
        if (++N4Intr::t_cnt[i] >= N4Intr::t_max[i]) {
            N4Intr::t_cnt[i] = 0;
            N4Intr::t_hit[i] = 1;
        }
    }
}
ISR(PCINT0_vect) { N4Intr::p_hit |= 1; }
ISR(PCINT1_vect) { N4Intr::p_hit |= 2; }
ISR(PCINT2_vect) { N4Intr::p_hit |= 4; }
#endif // ARDUINO

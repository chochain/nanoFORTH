///
/// \file nanoforth_core.cpp
/// \brief NanoForth Core Utility abstract class implementation
///
#include "nanoforth_core.h"
//
// tracing instrumentation
//
Stream *N4Core::_io{ &Serial };                          ///< default to Arduino Serial Monitor
U8      N4Core::_empty{ 1 };                             ///< empty flag for terminal input buffer

void N4Core::set_io(Stream *io) { _io = io; }
Stream *N4Core::get_io() { return _io; }

#if ARDUINO
#include <avr/pgmspace.h>
///
///> console input with cooperative threading
///
char N4Core::key()
{
    while (!_io->available()) NanoForth::yield();
    return _io->read();
}
///
///> console output single-char
///
void N4Core::d_chr(char c)     { _io->write(c); }
void N4Core::d_ptr(U8 *p)      { U16 a=(U16)p; d_chr('p'); d_adr(a); }
#else
int  Serial;                   // fake serial interface
char N4Core::key()             { return getchar(); }
void N4Core::d_chr(char c)     { printf("%c", c);  }
#endif //ARDUINO
void N4Core::d_nib(U8 n)       { d_chr((n) + ((n)>9 ? 'a'-10 : '0')); }
void N4Core::d_hex(U8 c)       { d_nib(c>>4); d_nib(c&0xf); }
void N4Core::d_adr(U16 a)      { d_nib((U8)(a>>8)&0xff); d_hex((U8)(a&0xff));   }
void N4Core::d_str(U8 *p)      { for (int i=0, sz=*p++; i<sz; i++) d_chr(*p++); }
//
// IO and Search Functions =================================================
///
///> emit a 16-bit integer
///
void N4Core::d_num(S16 n)
{
    if (n < 0) { n = -n; d_chr('-'); }        // process negative number
    U16 t = n/10;
    if (t) d_num(t);                          // recursively call higher digits
    d_chr('0' + (n%10));
}
///
///> dump byte-stream between pointers with delimiter option
///
void N4Core::d_mem(U8* base, U8 *p0, U16 sz, U8 delim)
{
	d_adr((U16)(p0 - base)); d_chr(':');
	for (int n=0; n<sz; n++) {
		if (delim && (n&0x3)==0) d_chr(delim);
		d_hex(*p0++);
	}
    d_chr(delim);
}
///
///> display the opcode name
/// 
void N4Core::d_name(U8 op, const char *lst, U8 space)
{
#if ARDUINO
    PGM_P p = reinterpret_cast<PGM_P>(lst)+1+op*3;
#else
    U8 *p = (U8*)lst+1+op*3;
#endif //ARDUINO
    char  c;
    d_chr(pgm_read_byte(p));
    if ((c=pgm_read_byte(p+1))!=' ' || space) d_chr(c);
    if ((c=pgm_read_byte(p+2))!=' ' || space) d_chr(c);
}
///
///> parse a literal from string
///
U8 N4Core::number(U8 *str, S16 *num)
{
    S16 n   = 0;
    U8  neg = (*str=='-') ? (str++, 1)  : 0;      // negative sign0;
    S16 base= (*str=='$') ? (str++, 16) : 10;

    for (; *str>='0'; str++) {
        if (base==10 && *str > '9') return 0;
        n *= base;
        n += (*str<='9') ? *str-'0' : (*str&0x5f)-'A'+10;
    }
    *num = neg ? -n : n;
    
    return 1;
}
///
///> check whether token available in input buffer
///
U8 N4Core::tib_empty()
{
    return _empty;
}
///
///> capture a token from console input buffer
///
U8 *N4Core::token(U8 trc, U8 clr)
{
    static U8 tib[TIB_SZ];
    static U8 *tp = tib;
    
	if (clr) {                                // clean input buffer
        _empty = 1;
        tp=tib;
        return 0;
    }
    if (tp==tib) _console_input(tib);         // buffer empty, read from console

    U8 *p = tp;                              // keep original tib pointer
    U8 sz = 0;
    while (*tp++!=' ') sz++;                 // advance to next word
    while (*tp==' ')   tp++;                // skip blanks

    if (*tp=='\r' || *tp=='\n') { tp=tib; _empty=1; }
    if (trc) {
        // debug info
        d_chr('\n');
        for (int i=0; i<5; i++) {
            d_chr(i<sz ? (*(p+i)<0x20 ? '_' : *(p+i)) : ' ');
        }
    }
    else if (tp==tib) d_chr('\n');
    
    return p;
}
///
///> search keyword in a NanoForth name field list
///
U8 N4Core::find(U8 *tkn, const char *lst, U16 *id)
{
    for (int n=1, m=pgm_read_byte(lst); n < m*3; n+=3) {
        if (tkn[0]==pgm_read_byte(lst+n) &&
            tkn[1]==pgm_read_byte(lst+n+1) &&
            (tkn[1]==' ' || tkn[2]==pgm_read_byte(lst+n+2))) {
            *id = n/3;
            return 1;
        }
    }
    return 0;
}
///
///> fill input buffer from console char-by-char til CR or LF hit
///
void N4Core::_console_input(U8 *tib)
{
    U8 *p = tib;
    for (;;) {
        char c = key();                      // get one char from input stream
        if (c=='\r' || c=='\n') {            // split on RETURN
            if (p > tib) {
                *p     = ' ';                // terminate input string
                *(p+1) = '\n';
                break;                       // skip empty token
            }
        }
        else if (c=='\b' && p > tib) {      // backspace
            *(--p) = ' ';
            d_chr(' ');
            d_chr('\b');
        }
        else if ((p - tib) >= (TIB_SZ-1)) {
            tx_str("TIB!\n");
            *p = '\n';
            break;
        }
        else *p++ = c;
    }
    _empty = (p==tib);
}    

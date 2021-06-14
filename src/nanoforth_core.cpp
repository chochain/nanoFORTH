///
/// \file nanoforth_core.cpp
/// \brief NanoForth Core Utility abstract class implementation
///
#include "nanoforth_core.h"
//
// tracing instrumentation
//
Stream *N4Core::_io{ &Serial };                ///< default to Arduino Serial Monitor
U8      N4Core::_empty{ 1 };                   ///< empty flag for terminal input buffer
U8      N4Core::_ucase{ 1 };                   ///< empty flag for terminal input buffer
U8      N4Core::_trc{ 0 };                     ///< tracing flag for debug output

void N4Core::set_io(Stream *io) { _io    = io; }
void N4Core::set_trace(U8 f)    { _trc   = f;  }
void N4Core::set_ucase(U8 uc)   { _ucase = uc; }
char N4Core::uc(char c)         { return (_ucase && (c>='A')) ? c&0x5f : c; }
U8   N4Core::is_tracing()       { return _trc; }

#if ARDUINO
#include <avr/pgmspace.h>
///
///> console input/output functions with cooperative threading
///
char N4Core::key()
{
    while (!_io->available()) NanoForth::yield();
    return _io->read();
}
void N4Core::d_chr(char c)     {
    Serial.print(c);
    _io->print(c);
    if (c=='\n') {
        _io->flush();
        NanoForth::yield();
    }
}
void N4Core::d_adr(U16 a)      { d_nib(a>>8); d_nib((a>>4)&0xf); d_nib(a&0xf); }
void N4Core::d_str(U8 *p)      { _io->print((char*)p); Serial.print((char*)p); }
void N4Core::d_ptr(U8 *p)      { U16 a=(U16)p; d_chr('p'); d_adr(a); }
void N4Core::d_num(S16 n)      { _io->print(n);        Serial.print(n); }
#else
int  Serial;                   // fake serial interface
char N4Core::key()             { return getchar();  }
void N4Core::d_chr(char c)     { printf("%c", c);   }
void N4Core::d_adr(U16 a)      { printf("%03x", a); }
void N4Core::d_str(U8 *p)      { printf("%s", p);   }
void N4Core::d_ptr(U8 *p)      { printf("%p", p);   }
void N4Core::d_num(S16 n)      { printf("%d", n);   }
#endif //ARDUINO
void N4Core::d_nib(U8 n)       { d_chr((n) + ((n)>9 ? 'a'-10 : '0')); }
void N4Core::d_u8(U8 c)        { d_nib(c>>4); d_nib(c&0xf); }
//
// IO and Search Functions =================================================
///
///
///> dump byte-stream between pointers with delimiter option
///
void N4Core::d_mem(U8* base, U8 *p0, U16 sz, U8 delim)
{
	d_adr((U16)(p0 - base)); d_chr(':');
	for (int n=0; n<sz; n++) {
		if (delim && (n&0x3)==0) d_chr(delim);
		d_u8(*p0++);
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
    U8  neg = (*str=='-') ? (str++, 1)  : 0;      /// * handle negative sign
    U8  base= (*str=='$') ? (str++, 16) : 10;     /// * handle hex number

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
U8 *N4Core::token(U8 clr)
{
    static U8 tib[TIB_SZ];
    static U8 *tp = tib;
    
	if (clr) {                               /// * optionally clean input buffer
        _empty = 1;
        tp=tib;
        return 0;
    }
    if (tp==tib) _console_input((U8*)tib);   /// * buffer empty, read from console (with trailing blank)

    U8 *p = (U8*)tp;                         /// * keep original tib pointer
    U8 sz = 0;
    while (*tp++!=' ') sz++;                 /// * advance to next word
    while (*tp==' ')   tp++;                 /// * skip blanks

    if (*tp=='\r' || *tp=='\n') { tp=tib; _empty=1; }   /// * end of input buffer
    if (_trc) {                              /// * optionally print token for debugging
        // debug info
        d_chr('\n');
        for (int i=0; i<5; i++) {
            d_chr(i<sz ? (*(p+i)<0x20 ? '_' : *(p+i)) : ' ');
        }
    }
    else if (tp==tib) d_chr('\n');
    
    return p;                                /// * return pointer to token
}
///
///> search keyword in a NanoForth name field list
///  * one blank byte padded at the end of input string
///
U8 N4Core::find(U8 *tkn, const char *lst, U16 *id)
{
    for (int n=1, m=pgm_read_byte(lst); n < m*3; n+=3) {
        if (uc(tkn[0])==pgm_read_byte(lst+n)   &&
            uc(tkn[1])==pgm_read_byte(lst+n+1) &&
            (tkn[1]==' ' || uc(tkn[2])==pgm_read_byte(lst+n+2))) {
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
        else if (c=='\b' && p > tib) {       // backspace
            *(--p) = ' ';
            d_chr(' ');
            d_chr('\b');
        }
        else if ((p - tib) >= (TIB_SZ-1)) {
            flash("TIB!\n");
            *p = '\n';
            break;
        }
        else *p++ = c;
    }
    _empty = (p==tib);
}    

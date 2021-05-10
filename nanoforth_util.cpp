///
/// \file nanoforth_util.cpp
/// \brief NanoForth Utility class implementation
///
#include <avr/pgmspace.h>
#include "nanoforth_util.h"
//
// tracing instrumentation
//
void N4Util::d_chr(char c)     { Serial.write(c);   }
void N4Util::d_nib(U8 n)       { d_chr((n) + ((n)>9 ? 'a'-10 : '0')); }
void N4Util::d_hex(U8 c)       { d_nib(c>>4); d_nib(c&0xf); }
void N4Util::d_adr(U16 a)      { d_nib((U8)(a>>8)&0xff); d_hex((U8)(a&0xff)); }
void N4Util::d_ptr(U8 *p)      { U16 a=(U16)p; d_chr('^'); d_adr(a); }
//
// IO and Search Functions =================================================
///
/// * emit a 16-bit integer
///
void N4Util::putnum(S16 n)
{
    if (n < 0) { n = -n; putchr('-'); }        // process negative number
    U16 t = n/10;
    if (t) putnum(t);                          // recursively call higher digits
    putchr('0' + (n%10));
}
///
/// * capture a token from console input buffer
///
U8 *N4Util::token(void)
{
    static U8 tib[TIB_SZ];
    static U8 *tp = tib;
    
    if (tp==tib) _console_input(tib);         // buffer empty, read from console

    U8 *p = tp;                               // keep original tib pointer
    U8 sz = 0;
    while (*tp++!=' ') sz++;                  // advance to next word
    while (*tp==' ')   tp++;                  // skip blanks

    if (*tp=='\r' || *tp=='\n') tp = tib;     // rewind input buffer

#if ASM_TRACE
    // debug info
    d_chr('\n');
    for (int i=0; i<5; i++) {
    	d_chr(i<sz ? (*(p+i)<0x20 ? '_' : *(p+i)) : ' ');
    }
#endif // ASM_TRACE
    return p;
}
///
/// * parse a literal from string
///
U8 N4Util::getnum(U8 *str, S16 *num)
{
    U8  neg = 0;
    S16 n   = 0;
    if (*str=='$') {
        for (str++; *str != ' '; str++) {    // hex number
            n *= 16;
            n += (*str<='9') ? *str-'0' : (*str&0x5f)-'A'+10;
        }
        *num = n;
        return 1;
    }
    if (*str=='-') { str++; neg=1; }         // negative sign
    if ('0' <= *str && *str <= '9') {        // decimal number
        for (n=0; *str != ' '; str++) {
            n *= 10;
            n += *str - '0';
        }
        *num = neg ? -n : n;
        return 1;
    }
    return 0;
}
///
/// * dump byte-stream with delimiter option
/// 
void N4Util::memdump(U8* base, U8 *p0, U16 sz, U8 delim)
{
	d_adr((U16)(p0 - base)); d_chr(':');
	for (int n=0; n<sz; n++, p0++) {
		if (delim && (n&0x3)==0) d_chr(delim);
		d_hex(*p0);
	}
    d_chr(delim);
}
///
/// * show a section of memory in Forth dump format
///
void N4Util::dump(U8* base, U8* p, U16 sz)
{
    putchr('\n');
    for (U16 i=0; i<sz; i+=0x20) {
        memdump(base, p, 0x20, ' ');
        putchr(' ');
        for (U8 j=0; j<0x20; j++, p++) {         // print and advance to next byte
            char c = *p & 0x7f;
            putchr((c==0x7f||c<0x20) ? '_' : c);
        }
        putchr('\n');
    }
}
///
/// * search keyword in a NanoForth name field list
///
U8 N4Util::find(U8 *tkn, const char *lst, U16 *id)
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
/// * fill input buffer from console char-by-char til CR or LF hit
///
void N4Util::_console_input(U8 *tib)
{
    U8 *p = tib;
    for (;;) {
        char c = NanoForth::key();
        if (c=='\r' || c=='\n') {            // split on RETURN
            if (p > tib) {
                *p     = ' ';                // terminate input string
                *(p+1) = '\n';
                break;                       // skip empty token
            }
        }
        else if (c=='\b' && p > tib) {       // backspace
            *(--p) = ' ';
            putchr(' ');
            putchr('\b');
        }
        else if ((p - tib) >= (TIB_SZ-1)) {
            putstr("TIB!\n");
            *p = '\n';
            break;
        }
        else *p++ = c;
    }
}    

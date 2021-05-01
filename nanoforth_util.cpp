#include <avr/pgmspace.h>
#include "nanoforth.h"
//
// tracing instrumentation
//
void d_chr(char c)     { Serial.write(c);   }
void d_nib(U8 n)       { d_chr((n) + ((n)>9 ? 'A'-10 : '0')); }
void d_hex(U8 c)       { d_nib(c>>4); d_nib(c&0xf); }
void d_adr(U16 a)      { d_nib((U8)(a>>8)&0xff); d_hex((U8)(a&0xff)); d_chr(':'); }
void d_ptr(U8 *p)      { U16 a = (U16)p; d_chr('^'); d_adr(a); }
//
// IO and Search Functions =================================================
//
void putmsg(__FlashStringHelper *msg) { Serial.print(msg); }
void putchr(char c)                   { Serial.write(c);   }
//
//  put a 16-bit integer
//
void putnum(S16 n)
{
    if (n < 0) { n = -n; putchr('-'); }
    U16 t = n/10;
    if (t) putnum(t);                          // recursively call higher digits
    putchr('0' + (n%10));
}
//
// fill buffer from console input
//
void _console_input(U8 *buf)
{
    U8 *p = buf;
    for (;;) {
        U8 c = getchr();
        if (c=='\r' || c=='\n') {            // split on RETURN
            if (p > buf) {
                *p     = ' ';                // terminate input string
                *(p+1) = '\n';
                break;                       // skip empty token
            }
        }
        else if (c=='\b' && p > buf) {       // backspace
            *(--p) = ' ';
            putchr(' ');
            putchr('\b');
        }
        else if ((p - buf) >= (BUF_SZ-1)) {
            putmsg(F("BUF\n"));
            *p = '\n';
            break;
        }
        else *p++ = c;
    }
}    
//
//  Get a Token
//
U8 *gettkn(void)
{
    static U8 buf[BUF_SZ], *bptr = buf;

    if (bptr==buf) _console_input(buf);         // buffer empty, read from console

    U8 *p0 = bptr;
    U8 sz  = 0;
    while (*bptr++!=' ') sz++;                  // advance to next word
    while (*bptr==' ') bptr++;                  // skip blanks

    if (*bptr=='\r' || *bptr=='\n') bptr = buf; // rewind buffer

#if ASM_TRACE
    // debug info
    d_chr('\n');
    for (U8 i=0; i<4; i++) {
    	d_chr(i<sz ? (*(p0+i)<0x20 ? '_' : *(p0+i)) : ' ');
    }
#endif // ASM_TRACE
    return p0;
}
//
// Process a Literal
//
U8 getnum(U8 *str, S16 *num)
{
    U8  neg = 0;
    S16 n   = 0;
    if (*str=='$') {
        for (str++; *str != ' '; str++) {
            n *= 16;
            n += *str - (*str<='9' ? '0' : 'A' - 10);
        }
        *num = n;
        return 1;
    }
    if (*str=='-') { str++; neg=1; }
    if ('0' <= *str && *str <= '9') {
        for (n=0; *str != ' '; str++) {
            n *= 10;
            n += *str - '0';
        }
        *num = neg ? -n : n;
        return 1;
    }
    return 0;
}
//
// memory dumper with delimiter option
// 
void dump(U8 *p0, U8 *p1, U8 d)
{
	U16 n = IDX(p0);
	d_adr(n);
	for (; p0<p1; n++, p0++) {
		if (d && (n&0x3)==0) d_chr(d);
		d_hex(*p0);
	}
}
//
// show  a section of dictionary memory
//
void showdic(U16 idx, U16 sz)            // idx: dictionary offset, sz: bytes
{
    U8 *p = PTR(idx & 0xfff0);           // 16-byte aligned
    sz &= 0xfff0;                        // 16-byte aligned
    putchr('\n');
    for (U8 i=0; i<sz; i+=0x20) {
        dump(p, p+0x20, ' ');
        putchr(' ');
        for (U8 j=0; j<0x20; j++, p++) { // print and advance to next byte
            U8 c = *p & 0x7f;
            putchr((c==0x7f||c<0x20) ? '_' : c);
        }
        putchr('\n');
    }
}
//
// scan the keyword through dictionary linked-list
//
U8 lookup(U8 *key, U16 *adr)
{
    for (U8 *p=last; p!=PTR(0xffff); p=PTR(GET16(p))) {
        if (p[2]==key[0] && p[3]==key[1] && (p[3]==' ' || p[4]==key[2])) {
            *adr = IDX(p);
            return 1;
        }
    }
    return 0;
}
//
// search keyword in a List
//
U8 find(U8 *key, const char *lst, U16 *id)
{
    for (U16 n=1, m=pgm_read_byte(lst); n < m*3; n+=3) {
        if (key[0]==pgm_read_byte(lst+n) &&
            key[1]==pgm_read_byte(lst+n+1) &&
            (key[1]==' ' || key[2]==pgm_read_byte(lst+n+2))) {
            *id = n/3;
            return 1;
        }
    }
    return 0;
}


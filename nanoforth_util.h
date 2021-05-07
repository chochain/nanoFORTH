#ifndef __SRC_NANOFORTH_UTIL_H
#define __SRC_NANOFORTH_UTIL_H
#include "nanoforth.h"

#define TIB_SZ     0x40

#define D_CHR(c)   N4Util::d_chr(c)
#define D_HEX(h)   N4Util::d_hex(h)
#define D_ADR(a)   N4Util::d_adr(a)

class N4Util                    // helper class
{
public:
    //
    // tracing instrumentation
    //
    static void d_chr(char c);  // print a char to console
    static void d_nib(U8 n);    // print a nibble
    static void d_hex(U8 c);    // print a 8-bit hex number
    static void d_adr(U16 a);   // print a 12-bit address
    static void d_ptr(U8 *p);   // print a pointer
    //
    // IO and Search Functions
    //
    static void putnum(S16 n);                   // sent a number literal to console
    static U8   getnum(U8 *str, S16 *num);       // process a literal
    static U8   *token(void);                    // get a token from console input
    //
    // memory dummpers
    //
    static void memdump(U8 *p0, U8 *p1, U8 d);   // show memory content between pointers (d: delimiter option)
    static void dump(U8 *p, U16 sz);             // dump memory block with dictionary offset and length
    //
    // string list scanners
    //
    static U8   find(U8 *tkn, const char *lst, U16 *id);      // find(token) in string list
    
private:
    static void _console_input(U8 *tib);
};
#endif //__SRC_NANOFORTH_UTIL_H


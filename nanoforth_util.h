///
/// \file nanoforth_util.h
/// \brief NanoForth Utility class
///
#ifndef __SRC_NANOFORTH_UTIL_H
#define __SRC_NANOFORTH_UTIL_H
#include "nanoforth.h"

#define TIB_SZ         0x40              /**< console(terminal) input buffer size */
//
// Serial IO macros
//
/* for debugging
#define PROGMEM
#define pgm_read_byte(p)  (*(p))
#define millis()          0
#define putstr(msg)    printf("%s", msg)
#define putchr(c)      printf("%c", c)
#define puthex(v)      printf("%02x", v)
*/
#define putstr(msg)    Serial.print(F(msg))
#define putchr(c)      Serial.write((char)c)
#define puthex(v)      Serial.print((U16)v, HEX)
//
// memory access opcodes
/// \def SET8
/// \brief 1-byte write
/// \def SET16
/// \brief 2-byte write, prevent alignment issue (on 32-bit CPU) and preseve Big-Endian encoding
/// \def GET16
/// \brief 2-byte read, prevent aligntment issue (on 32-bit CPU) and preseve Big-Endian encoding
///
#define SET8(p, c)     (*(U8*)(p)++=(U8)(c))
#define SET16(p, n)    do { U16 x=(U16)(n); SET8(p,(x)>>8); SET8(p,(x)&0xff); } while(0)
#define GET16(p)       (((U16)(*(U8*)(p))<<8) + *((U8*)(p)+1))
//
// common console output macros
//
#define D_CHR(c)       N4Util::d_chr(c)
#define D_HEX(h)       N4Util::d_hex(h)
#define D_ADR(a)       N4Util::d_adr(a)
///
/// NanoForth helper class
///
class N4Util
{
public:
    //
    // tracing instrumentation
    //
    static void d_chr(char c);             ///< print a char to console
    static void d_nib(U8 n);               ///< print a nibble
    static void d_hex(U8 c);               ///< print a 8-bit hex number
    static void d_adr(U16 a);              ///< print a 12-bit address
    static void d_ptr(U8 *p);              ///< print a pointer
    //
    // IO and Search Functions
    //
    static void putnum(S16 n);             ///< sent a number literal to console
    static U8   getnum(U8 *str, S16 *num); ///< process a literal from string given
    static U8   *token(void);              ///< get a token from console input
    //
    // memory dummpers
    //
    static void memdump(                   ///< mem between pointers (d: delimiter option)
        U8 *base,                          ///< reference memory pointer (start of dictionary)
        U8 *p0,                            ///< starting memory pointer
        U16 sz,                            ///< number of bytes to print
        U8 delim                           ///< delimiter, ' ' for space, 0 for none
        );
    static void dump(                      ///< mem block with dictionary offset and length
        U8 *base,                          ///< reference memory pointer (start of dictionary)
        U8 *p,                             ///< starting memory pointer
        U16 sz                             ///< number of bytes to print
        );
    ///
    /// find token in string list
    ///
    static U8   find(
        U8 *tkn,                           ///< token to be found
        const char *lst,                   ///< string list to be scanned
        U16 *id                            ///< resultant index if found
        );
    
private:
    static void _console_input(U8 *tib);
};
#endif //__SRC_NANOFORTH_UTIL_H


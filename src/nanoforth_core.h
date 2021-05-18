///
/// \file nanoforth_core.h
/// \brief NanoForth Core Utility abstract class
///
#ifndef __SRC_NANOFORTH_CORE_H
#define __SRC_NANOFORTH_CORE_H
#include "nanoforth.h"

constexpr U16 TIB_SZ   = 0x40;             /**< console(terminal) input buffer size */
constexpr U8  TIB_CLR  = 0x1;
//
// Serial IO macros
//
#if ARDUINO
#define putstr(msg)    Serial.print(F(msg))
#define puthex(v)      Serial.print((U16)v, HEX)
#else
#define putstr(msg)    printf("%s", msg)
#define puthex(v)      printf("%02x", v)
#endif // ARDUINO
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
///
/// NanoForth Core Helper abstract class
///
class N4Core
{
    static Stream &_io;                    ///< io stream (static member)
    static U8   _empty;                    ///< token ininput buffer empty flag
    
public:
    static void set_io(Stream &io);        ///< initialize or redirect io stream
    static char key();                     ///< Arduino's Serial.getchar(), yield to user tasks when waiting
    //
    // dot_* for console output routines
    //
    static void d_chr(char c);             ///< print a char to console
    static void d_nib(U8 n);               ///< print a nibble
    static void d_hex(U8 c);               ///< print a 8-bit hex number
    static void d_adr(U16 a);              ///< print a 12-bit address
    static void d_str(U8 *p);              ///< handle dot string (byte-stream leading with length)
    static void d_ptr(U8 *p);              ///< print a pointer
    static void d_num(S16 n);              ///< sent a number literal to console
    static void d_mem(                     ///< mem between pointers (d: delimiter option)
        U8 *base,                          ///< reference memory pointer (start of dictionary)
        U8 *p0,                            ///< starting memory pointer
        U16 sz,                            ///< number of bytes to print
        U8 delim                           ///< delimiter, ' ' for space, 0 for none
        );
    static void d_name(U8 op, const char *lst, U8 space);    ///< display opcode 3-char name
    //
    // Search Functions
    //
    static U8   tib_empty();               ///< check input buffer
    static U8   *token(U8 trc, U8 clr=0);  ///< get a token from console input
    static U8   number(U8 *str, S16 *num); ///< process a literal from string given
    ///
    /// find token in string list
    ///
    static U8   find(
        U8 *tkn,                           ///< token to be found
        const char *lst,                   ///< string list to be scanned
        U16 *id                            ///< resultant index if found
        );
    
private:
    static void _console_input(U8 *tib);   ///< retrieve input stream from console
};
#endif //__SRC_NANOFORTH_CORE_H


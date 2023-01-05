/**
 * @file nanoforth_core.h
 * @brief nanoForth Core Utility abstract class
 */
#ifndef __SRC_NANOFORTH_CORE_H
#define __SRC_NANOFORTH_CORE_H
#include "nanoforth.h"

#if ARDUINO
#define show(s)      { _io->print(F(s)); _io->flush(); }
#define INLINE       __attribute__((always_inline))
#else
#define show(s)      log(s)
#define INLINE       __attribute__((always_inline))
#endif // ARDUINO

///@name Memory Access Ops
///
/// @def SET8
/// @brief 1-byte write
/// @def SET16
/// @brief 2-byte write, prevent alignment issue (on 32-bit CPU) and preserve Big-Endian encoding
/// @def GET16
/// @brief 2-byte read, prevent alignment issue (on 32-bit CPU) and preserve Big-Endian encoding
///@{
#define SET8(p, c)     (*(U8*)(p)++=(U8)(c))
#define SET16(p, n)    do { U16 x=(U16)(n); SET8(p,(x)>>8); SET8(p,(x)&0xff); } while(0)
#define GET16(p)       (((U16)(*(U8*)(p))<<8) + *((U8*)(p)+1))
///@}
///
/// nanoForth Core Helper abstract class
///
class N4Core
{
    static U8   _empty;                    ///< token input buffer empty flag
    static U8   _ucase;                    ///< case insensitive
    static U8   _hex;                      ///< numeric radix for display
    static U8   _trc;                      ///< tracing flags

protected:
    static U8     *dic;                    ///< base of dictionary
    static U16    *rp;					   ///< base of return stack
    static S16    *sp;                     ///< top of data stack
    static U8     *tib;                    ///< base of terminal input buffer
    static Stream *_io;                    ///< IO stream (static member)

public:
    static void set_mem(U8 *mem, U16 msz, U16 ssz) {
        dic = mem;                         /// * start of dictionary
        rp  = (U16*)(mem+msz);             /// * grows toward sp
        sp  = (S16*)(mem+msz+ssz);         /// * grows toward 0
        tib = (U8*)sp;                     /// * grows toward max
    }
    static void set_io(Stream *io) { _io = io; }    ///< initialize or redirect IO stream
    static void set_hex(U8 f)      { _hex = f; }    ///< enable/disable hex numeric radix
    static void set_trace(U8 f)    { _trc = f; }    ///< enable/disable execution tracing
    static U8   is_tracing()       { return _trc; } ///< return tracing flag
    static void set_ucase(U8 uc)   { _ucase = uc; } ///< set case sensitiveness
    static char uc(char c)         {                ///< upper case for case-insensitive matching
        return (_ucase && (c>='A')) ? c&0x5f : c;
    }
    ///
    ///@name dot_* for Console Input/Output Routines
    ///@{
    static char key();                     ///< Arduino's Serial.getchar(), yield to user tasks when waiting
    static void d_chr(char c);             ///< print a char to console
    static void d_adr(U16 a);              ///< print a 12-bit address
    static void d_str(U8 *p);              ///< handle dot string (byte-stream leading with length)
    static void d_ptr(U8 *p);              ///< print a pointer
    static void d_nib(U8 n);               ///< print a nibble
    static void d_u8(U8 c);                ///< print a 8-bit hex number
    static void d_num(S16 n);              ///< sent a number literal to console
    static void d_mem(                     ///< display memory block
        U8 *base,                          ///< reference memory pointer (start of dictionary)<br/>
        U8 *p0,                            ///< starting memory pointer<br/>
        U16 sz,                            ///< number of bytes to print<br/>
        U8 delim                           ///< delimiter, ' ' for space, 0 for none
        );
    static void d_name(                    ///< display opcode 3-char name
        U8 op,                             ///< opcode
        const char *lst,                   ///< nanoForth string formatted list
        U8 space                           ///< delimiter to append at the end
        );
    ///@}
    ///
    ///@name Search Functions
    ///@{
    static U8   is_tib_empty();            ///< check whether input buffer is empty
    static void clear_tib();               ///< reset input buffer
    static U8   *get_token(bool rst=false);///< get a token from console input
    static U8   number(                    ///< process a literal from string given
        U8 *tkn,                           ///< token string of a number
        S16 *num                           ///< number pointer for return value
        );
    ///
    /// scan token from a given string list
    ///
    static U8   scan(
        U8 *tkn,                           ///< token to be searched
        const char *lst,                   ///< string list to be scanned
        U16 *id                            ///< resultant index if found
        );
    ///@}

private:
    static void _console_input();          ///< retrieve input stream from console
};
#endif //__SRC_NANOFORTH_CORE_H

/**
 * @file
 * @brief nanoForth Core Utilities
 *        + memory and IO helper functions
 */
#ifndef __SRC_N4_CORE_H
#define __SRC_N4_CORE_H
#include "n4.h"
///
///@name Default Heap sizing
///@{
constexpr U16 N4_DIC_SZ = 0x400;  /**< default dictionary size             */
constexpr U16 N4_STK_SZ = 0x80;   /**< default parameter/return stack size */
constexpr U16 N4_TIB_SZ = 0x80;
///@}
#if ARDUINO
#define show(s)      { io->print(F(s)); io->flush(); }
#else
#define show(s)      log(s)
#endif // ARDUINO
///
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
/// nanoForth memory and IO helper functions
///
namespace N4Core
{
    extern Stream *io;              ///< default to Arduino Serial Monitor
    extern U8     *dic;             ///< base of dictionary
    extern U16    *rp;				///< base of return stack
    extern S16    *sp;              ///< top of data stack
    extern U8     trc;              ///< tracing flag

    void init_mem();                ///< initialize MMU
    void memstat();                 ///< display MMU statistics

    void set_pre(const char *code); ///< set preload Forth code
    void set_io(Stream *s);         ///< initialize or redirect IO stream
    void set_hex(U8 f);             ///< enable/disable hex numeric radix
    void set_ucase(U8 uc);          ///< set case sensitiveness
    char uc(char c);
    ///
    ///@name dot_* for Console Input/Output Routines
    ///@{

    char key();                     ///< Arduino's Serial.getchar(), yield to user tasks when waiting
    void d_chr(char c);             ///< print a char to console
    void d_adr(U16 a);              ///< print a 12-bit address
    void d_str(U8 *p);              ///< handle dot string (byte-stream leading with length)
    void d_ptr(U8 *p);              ///< print a pointer
    void d_nib(U8 n);               ///< print a nibble
    void d_u8(U8 c);                ///< print a 8-bit hex number
    void d_num(S16 n);              ///< sent a number literal to console
    void d_mem(                     ///< display memory block
        U8 *base,                   ///< reference memory pointer (start of dictionary)<br/>
        U8 *p0,                     ///< starting memory pointer<br/>
        U16 sz,                     ///< number of bytes to print<br/>
        U8 delim                    ///< delimiter, ' ' for space, 0 for none
        );
    void d_name(                    ///< display opcode 3-char name
        U8 op,                      ///< opcode
        const char *lst,            ///< nanoForth string formatted list
        U8 space                    ///< delimiter to append at the end
        );
    ///@}
    ///
    ///@name Input buffer Functions
    ///@{
    void clear_tib();               ///< reset input buffer
    U8   ok();                      ///< check whether input buffer is empty
    U8   *get_token(bool rst=false);///< get a token from console input
    U8   number(                    ///< process a literal from string given
        U8 *tkn,                    ///< token string of a number
        S16 *num                    ///< number pointer for return value
        );
    ///
    /// scan token from a given string list
    ///
    U8  scan(                       ///< find token in given string list
        U8 *tkn,                    ///< token to be searched
        const char *lst,            ///< string list to be scanned
        U16 *id                     ///< resultant index if found
        );
    ///@}
};
#endif //__SRC_N4_CORE_H

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
constexpr U16 N4_TIB_SZ = 0x80;   /**< terminal input buffer size          */
constexpr U16 N4_RAM_SZ = N4_DIC_SZ + N4_STK_SZ + N4_TIB_SZ;
///@}
#if ARDUINO
#define show(s)      { io->print(F(s)); io->flush(); }
#else
#define show(s)      log(s)
#endif // ARDUINO
///
///@name Logical Memory Access Ops
///
/// @def ENC8
/// @brief 1-byte write (Note: increment calling pointer by 1)
/// @def ENCA
/// @brief instruction unit write, prevent alignment issue (on 32-bit CPU) and preserve Big-Endian encoding
/// @def GETA
/// @brief instruction unit read
/// @def STORE
/// @brief data unit store, same as ENC for 16-bit Forth (physical vs logical)
/// @def FETCH
/// @brief DU fetch
///@{
#define ENC8(p, c)  (*(U8*)(p)++=(U8)(c))
#define ENCA(p, a)  { IU x=(IU)(a); ENC8(p,(x)>>8); ENC8(p,(x)&0xff); }
#define GETA(p)     (((IU)(*(U8*)(p))<<8) + *((U8*)(p)+1))
#define STORE(p, v) { DU x=(DU)(v); ENC8(p,(x)>>8); ENC8(p,(x)&0xff); }
#define FETCH(p)    (((DU)(*(U8*)(p))<<8) + *((U8*)(p)+1))
///@}
///
/// nanoForth memory and IO helper functions
///
typedef struct {
    IU    *rp;                      ///< base of return stack
    DU    *sp;                      ///< top of data stack
} N4Task;

namespace N4Core
{
    extern N4Task vm;               ///< VM state
    extern U8     dic[];            ///< base of dictionary
    extern U8     trc;              ///< tracing flag
    extern Stream *io;              ///< default to Arduino Serial Monitor

    void mstat();                   ///< display MMU statistics

    void set_pre(const char *code); ///< set embedded Forth code
    void set_io(Stream *s);         ///< initialize or redirect IO stream
    void set_hex(U8 f);             ///< enable/disable hex numeric radix
    void set_ucase(U8 uc);          ///< set case sensitiveness
    char uc(char c);
    ///
    ///@name dot_* for Console Input/Output Routines
    ///@{
    char key();                     ///< Arduino's Serial.getchar(), yield to user tasks when waiting
    void d_chr(char c);             ///< print a char to console
    void d_adr(IU a);               ///< print a 12-bit address
    void d_str(U8 *p);              ///< handle dot string (byte-stream leading with length)
    void d_ptr(U8 *p);              ///< print a pointer
    void d_nib(U8 n);               ///< print a nibble
    void d_u8(U8 c);                ///< print a 8-bit hex number
    void d_num(DU n);               ///< sent a number literal to console
    void d_pin(U16 p, U16 v);       ///< set pin a given pinMode (INPUT, OUTPUT)
    U16  d_in(U16 p);               ///< fetch from GPIO port
    void d_out(U16 p, U16 v);       ///< send output to GPIO ports
    void d_mem(                     ///< display memory block
        U8 *base,                   ///< reference memory pointer (start of dictionary)<br/>
        U8 *p0,                     ///< starting memory pointer<br/>
        IU sz,                      ///< number of bytes to print<br/>
        U8 delim                    ///< delimiter, ' ' for space, 0 for none
        );
    void d_name(U8 *p);             ///< print 3 characters name
    void d_name(                    ///< display opcode & 3-char name
        U8 op,                      ///< opcode
        const char *lst,            ///< nanoForth string formatted list
        U8 space                    ///< delimiter to append at the end
        );
    U16  a_in(U16 p);               ///< fetch from analog port
    void a_out(U16 p, U16 v);       ///< send output to GPIO ports
    ///@}
    ///
    ///@name Input buffer Functions
    ///@{
    void clear_tib();               ///< reset input buffer
    U8   ok();                      ///< check whether input buffer is empty
    U8   *get_token(U8 rst=0);      ///< get a token from console input
    U8   number(                    ///< process a literal from string given
        U8 *tkn,                    ///< token string of a number
        DU *num                     ///< number pointer for return value
        );
    ///
    /// scan token from a given string list
    ///
    U8  scan(                       ///< find token in given string list
        U8 *tkn,                    ///< token to be searched
        const char *lst,            ///< string list to be scanned
        IU *id                      ///< resultant index if found
        );
    ///@}
};
#endif //__SRC_N4_CORE_H

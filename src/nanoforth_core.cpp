/**
 * @file nanoforth_core.cpp
 * @brief nanoForth Core Utility abstract class implementation
 *
 */
#include "nanoforth_core.h"
///
///@name VM static variables
///@{
U8     *N4Core::dic    { NULL };               ///< base of dictionary
U16    *N4Core::rp     { NULL };               ///< base of return stack
S16    *N4Core::sp     { NULL };               ///< top of data stack
U8     *N4Core::tib    { NULL };               ///< base of terminal input buffer
///@}
///@name IO controls
///@{
Stream *N4Core::_io    { &Serial };            ///< default to Arduino Serial Monitor
U8      N4Core::_hex   { 0 };                  ///< numeric radix for display
U8      N4Core::_empty { 1 };                  ///< empty flag for terminal input buffer
U8      N4Core::_ucase { 1 };                  ///< empty flag for terminal input buffer
U8      N4Core::_trc   { 0 };                  ///< tracing flag for debug output
///@}
///@name Console IO Functions with Cooperative Threading support
///@{
#if ARDUINO
#include <avr/pgmspace.h>
///
///> char input from console
///
char N4Core::key()
{
    while (!_io->available()) NanoForth::yield();
    return _io->read();
}
void N4Core::d_chr(char c)     {
    _io->print(c);
    if (c=='\n') {
        _io->flush();
        NanoForth::yield();
    }
}
void N4Core::d_adr(U16 a)      { d_nib(a>>8); d_nib((a>>4)&0xf); d_nib(a&0xf); }
void N4Core::d_ptr(U8 *p)      { U16 a=(U16)p; d_chr('p'); d_adr(a); }
void N4Core::d_num(S16 n)      { _hex ? _io->print(n,HEX) : _io->print(n); }
#else
int  Serial;                   // fake serial interface
char N4Core::key()             { return getchar();  }
void N4Core::d_chr(char c)     { printf("%c", c);   }
void N4Core::d_adr(U16 a)      { printf("%03x", a); }
void N4Core::d_ptr(U8 *p)      { printf("%p", p);   }
void N4Core::d_num(S16 n)      { printf(_hex ? "%x" : "%d", n); }
#endif //ARDUINO
void N4Core::d_str(U8 *p)      { for (U8 i=0, sz=*p++; i<sz; i++) d_chr(*p++); }
void N4Core::d_nib(U8 n)       { d_chr((n) + ((n)>9 ? 'a'-10 : '0')); }
void N4Core::d_u8(U8 c)        { d_nib(c>>4); d_nib(c&0xf); }
///@}
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
    U8  neg = (*str=='-') ? (str++, 1)  : 0;  /// * handle negative sign
    U8  base= _hex ? 16 : 10;                 /// * handle hex number

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
U8 N4Core::is_tib_empty()
{
    return _empty;
}
///
///> clear terminal input buffer
///
void N4Core::clear_tib() {
    get_token(true);                         ///> empty the static tib inside #get_token
}
///
///> capture a token from console input buffer
///
U8 *N4Core::get_token(bool rst)
{
    static U8 *tp = tib;                     ///> token pointer to input buffer
    static U8 dq  = 0;                       ///> dot_string flag

    if (rst) { tp = tib; _empty = 1; return 0; }  /// * reset TIB for new input
    while (_empty || *tp==0 || *tp=='\\') {
        _console_input();                    ///>  read from console (with trailing blank)
        while (*tp==' ') tp++;               ///>  skip leading spaces
    }
    if (!dq) {
        while (*tp=='(' && *(tp+1)==' ') {   /// * handle ( ...) comment, TODO: multi-line
            while (*tp && *tp++!=')');       ///> find the end of comment
            while (*tp==' ') tp++;           ///> skip trailing spaces
        }
    }
    U8 *p = (U8*)tp;
    U8 cx = dq ? '"' : ' ';                  /// * set delimiter
    U8 sz = 0;
    while (*tp && *tp!='(' && *tp++!=cx) sz++;/// * count token length
    if (_trc) {                              /// * optionally print token for debugging
        d_chr('\n');
        for (int i=0; i<5; i++) {
            d_chr(i<sz ? (*(p+i)<0x20 ? '_' : *(p+i)) : ' ');
        }
    }
    while (*tp==' ') tp++;                   /// * skip spaces, advance pointer to next token
    if (*tp==0 || *tp=='\\') { tp = tib; _empty = 1; }

    dq  = (*p=='.' && *(p+1)=='"');          /// * flag token was dot_string

    return p;                                /// * return pointer to token
}
///
///> search keyword in a nanoForth name field list
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
void N4Core::_console_input()
{
    U8 *p = tib;
    d_chr('\n');
    for (;;) {
        char c = key();                      // get one char from input stream
        if (c=='\r' || c=='\n') {            // split on RETURN
            if (p > tib) {
                *p     = ' ';                // pad extra space (in case word is 1-char)
                *(p+1) = 0;                  // terminate input string
                break;                       // skip empty token
            }
        }
        else if (c=='\b' && p > tib) {       // backspace
            *(--p) = ' ';
            d_chr(' ');
            d_chr('\b');
        }
        else if (p > (U8*)(&c - 0x10)) {     // address of auto variable
            show("TIB!\n");
            *p = 0;
            break;
        }
        else *p++ = c;
    }
    _empty = (p==tib);
}

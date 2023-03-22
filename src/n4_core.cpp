/**
 * @file
 * @brief nanoForth Core Utilities
 *        - low level memory and IO management
 */
#include "n4_core.h"

#if !ARDUINO
int  Serial;           				     ///< fake serial interface
#endif // !ARDUINO

namespace N4Core {
///
///@name MMU controls
///@{
U8     *dic    { NULL };                       ///< base of dictionary
U16    *rp     { NULL };                       ///< base of return stack
S16    *sp     { NULL };                       ///< top of data stack
///@}
///@name IO controls
///@{
Stream  *io    { &Serial };                    ///< default to Arduino Serial Monitor
U8      trc    { 0 };                          ///< tracing flag
char    *_pre  { NULL };                       ///< preload Forth code
U8      *_tib  { NULL };                       ///< base of terminal input buffer
U8      _hex   { 0 };                          ///< numeric radix for display
U8      _empty { 1 };                          ///< empty flag for terminal input buffer
U8      _ucase { 0 };                          ///< empty flag for terminal input buffer
///@}
///
void init_mem() {
    U16 sz = N4_DIC_SZ + N4_STK_SZ + N4_TIB_SZ;///< core memory block
    dic  = (U8*)malloc(sz);                    /// * allocate Forth memory block
    _tib = dic + N4_DIC_SZ + N4_STK_SZ;        /// * grows N4_TIB_SZ
}
void set_pre(const char *code) { _pre = (char*)code; }
void set_io(Stream *s)  { io   = s; }          ///< initialize or redirect IO stream
void set_hex(U8 f)      { _hex = f; }          ///< enable/disable hex numeric radix
void set_ucase(U8 uc)   { _ucase = uc; }       ///< set case sensitiveness
char uc(char c)      {                         ///< upper case for case-insensitive matching
    return (_ucase && (c>='A')) ? c&0x5f : c;
}
///
///> show system memory allocation info
///
void memstat()
{
#if ARDUINO && TRC_LEVEL > 0
    S16 bsz = (S16)((U8*)&bsz - _tib);                        // free for TIB in bytes
    show("mem=");    d_ptr(dic);
    show("[dic=$");  d_adr(N4_DIC_SZ);
    show("|stk=$");  d_adr(N4_STK_SZ);
    show("|tib=$");  d_adr(N4_TIB_SZ);
    show("] auto="); d_num((U16)((U8*)&bsz - &_tib[N4_TIB_SZ]));
#else
    log("MEM=$");    logx(N4_DIC_SZ + N4_STK_SZ + N4_TIB_SZ); // forth memory block
    log("[DIC=$");   logx(N4_DIC_SZ);                         // dictionary size
    log("|STK=$");   logx(N4_STK_SZ);                         // stack size
    log("|TIB=$");   logx(N4_TIB_SZ);
#endif // ARDUINO
    show("]\n");
}
///@}
///@name Console IO Functions with Cooperative Threading support
///@{
#if ARDUINO
#include <avr/pgmspace.h>
///
///> char IO from console i.e. RX/TX
///
char key()
{
    while (!io->available()) NanoForth::yield();
    return io->read();
}
void d_chr(char c)     {
    io->print(c);
    if (c=='\n') {
        io->flush();
        NanoForth::yield();
    }
}
void d_adr(U16 a)      { d_nib(a>>8); d_nib((a>>4)&0xf); d_nib(a&0xf); }
void d_ptr(U8 *p)      { U16 a=(U16)p; d_chr('p'); d_adr(a); }
void d_num(S16 n)      { _hex ? io->print(n&0xffff,HEX) : io->print(n); }
void d_out(U16 p, U16 v) {
    switch (p & 0x300) {
    case 0x100:            // PORTD (0~7)
        DDRD  = DDRD | (p & 0xfc);  /// * mask out RX,TX
        PORTD = (U8)(v & p) | (PORTD & ~p);
        break;
    case 0x200:            // PORTB (8~13)
        DDRB  = DDRB | (p & 0xff);
        PORTB = (U8)(v & p) | (PORTB & ~p);
        break;
    case 0x300:            // PORTC (A0~A6)
        DDRC  = DDRC | (p & 0xff);
        PORTC = (U8)(v & p) | (PORTC & ~p);
        break;
    default: digitalWrite(p, v);
    }
}
#else
char key()             { return getchar();  }
void d_chr(char c)     { printf("%c", c);   }
void d_adr(U16 a)      { printf("%03x", a); }
void d_ptr(U8 *p)      { printf("%p", p);   }
void d_num(S16 n)      { printf(_hex ? "%x" : "%d", n); }
void d_out(U16 p, U16 v) { /* do nothing */ }
#endif //ARDUINO
void d_str(U8 *p)      { for (U8 i=0, sz=*p++; i<sz; i++) d_chr(*p++); }
void d_nib(U8 n)       { d_chr((n) + ((n)>9 ? 'a'-10 : '0')); }
void d_u8(U8 c)        { d_nib(c>>4); d_nib(c&0xf); }
///@}
///
///> dump byte-stream between pointers with delimiter option
///
void d_mem(U8* base, U8 *p0, U16 sz, U8 delim)
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
void d_name(U8 op, const char *lst, U8 space)
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
U8 number(U8 *str, S16 *num)
{
    S16 n   = 0;
    U8  c   = *str;
    U8  neg = (c=='-') ? (c=*++str, 1)  : 0;              /// * handle negative sign
    U8  base= c=='$' ? (str++, 16) : (_hex ? 16 : 10);    /// * handle hex number

    while ((c=*str++) >= '0') {
        n *= base;
        if (base==10 && c > '9') return 0;
        if (c <= '9') n += c - '0';
        else {
            c &= 0x5f;
            if (c < 'A' || c > 'F') return 0;
            n += c - 'A' + 10;
        }
    }
    *num = neg ? -n : n;

    return 1;
}
///
///> clear terminal input buffer
///
void clear_tib() {
    get_token(true);                         ///> empty the static tib inside #get_token
}
///
///> fill input buffer from console char-by-char til CR or LF hit
///
char vkey() {
	static char *p = _pre;                   /// capture preload Forth code
#if ARDUINO
    char c = pgm_read_byte(p);
#else
    char c = *p;
#endif // ARDUINO
	return c ? (p++, c) : key();                /// feed key() after preload exhausted
}

void _console_input()
{
    U8 *p = _tib;
    d_chr('\n');
    for (;;) {
        char c = vkey();                     /// * get one char from input stream
        if (c=='\r' || c=='\n') {            /// * split on RETURN
            if (p > _tib) {
                *p     = ' ';                /// * pad extra space (in case word is 1-char)
                *(p+1) = 0;                  /// * terminate input string
                break;                       /// * skip empty token
            }
        }
        else if (c=='\b' && p > _tib) {      /// * backspace
            *(--p) = ' ';
            d_chr(' ');
            d_chr('\b');
        }
        else if (p > (U8*)(&c - sizeof(U32))) { /// * prevent buffer overrun (into auto vars)
            show("TIB!\n");
            *p = 0;
            break;
        }
        else *p++ = c;
    }
    _empty = (p==_tib);
}
///
///> display OK prompt if input buffer is empty
///
U8 ok()
{
	if (_empty) {
		///
		///> console prompt with stack dump
		///
		S16 *s0 = (S16*)_tib;                /// * fetch top of heap
	    if (sp > s0) {                       /// * check stack overflow
	        show("OVF!\n");
	        sp = s0;                         // reset to top of stack block
	    }
	    for (S16 *p=s0-1; p >= sp; p--) {    /// * dump stack content
	        d_num(*p); d_chr('_');
	    }
	    show("ok");                          /// * user input prompt
	}
    return _empty;
}
///
///> capture a token from console input buffer
///
U8 *get_token(bool rst)
{
    static U8 *tp = _tib;                    ///> token pointer to input buffer
    static U8 dq  = 0;                       ///> dot_string flag

    if (rst) { tp = _tib; _empty = 1; return 0; }  /// * reset TIB for new input
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
    if (trc) {                               /// * optionally print token for debugging
        d_chr('\n');
        for (int i=0; i<5; i++) {
            d_chr(i<sz ? (*(p+i)<0x20 ? '_' : *(p+i)) : ' ');
        }
    }
    while (*tp==' ') tp++;                   /// * skip spaces, advance pointer to next token
    if (*tp==0 || *tp=='\\') { tp = _tib; _empty = 1; }

    dq  = (*p=='.' && *(p+1)=='"');          /// * flag token was dot_string

    return p;                                /// * return pointer to token
}
///
///> search keyword in a nanoForth name field list
///  * one blank byte padded at the end of input string
///
U8 scan(U8 *tkn, const char *lst, U16 *id)
{
    for (int n=1, m=pgm_read_byte(lst); n < m*3; n+=3) {
        if (uc(tkn[0])==pgm_read_byte(lst+n)   &&
            uc(tkn[1])==pgm_read_byte(lst+n+1) &&
            (tkn[1]==' ' || uc(tkn[2])==pgm_read_byte(lst+n+2))) {
            *id = n/3;  // 3-char a word
            return 1;
        }
    }
    return 0;
}

} // namespace N4Core

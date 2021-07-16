/**
 * @file nanoforth_asm.cpp
 * @brief nanoForth Assmebler implementation
 *
 * ####Assembler Memory Map:
 * 
 * @code
 *    mem[...dic_sz...[...stk_sz...]
 *       |                         |
 *       +-dic-->            <--rp-+
 * @endcode
 */
#include "nanoforth_asm.h"
#if ARDUINO
#include <EEPROM.h>
#endif //ARDUINO
///
///@name nanoForth built-in vocabularies
///
/// @var CMD
/// @brief words for interpret mode.
/// @var JMP (note: ; is hardcoded at position 0, do not change it)
/// @brief words for branching op in compile mode.
/// @var PRM
/// @brief primitive words (45 allocated, 64 max).
/// @var PMX
/// @brief loop control opcodes
///
///@{
PROGMEM const char CMD[] = "\x06" \
    ":  " "VAR" "CST" "FGT" "DMP" "BYE";
PROGMEM const char JMP[] = "\x0b" \
    ";  " "IF " "ELS" "THN" "BGN" "UTL" "WHL" "RPT" "FOR" "NXT" \
    "I  ";
PROGMEM const char PRM[] = "\x31" \
    "DRP" "DUP" "SWP" "OVR" "ROT" "+  " "-  " "*  " "/  " "MOD" \
    "NEG" "AND" "OR " "XOR" "NOT" "=  " "<  " ">  " "<= " ">= " \
    "<> " "@  " "!  " "C@ " "C! " "KEY" "EMT" "CR " ".  " ".\" "\
    ">R " "R> " "WRD" "HRE" "CEL" "ALO" "SAV" "LD " "TRC" "CLK" \
    "D+ " "D- " "DNG" "DLY" "IN " "AIN" "OUT" "PWM" "PIN";
PROGMEM const char PMX[] = "\x4" \
    "FOR" "NXT" "BRK" "I  ";
constexpr U16 OP_SEMI = 0;                           /**< semi-colon, end of function definition */
///@}
///
///@name Branching
///@{
#define JMP000(p,j)     SET16(p, (j)<<8)
#define JMPSET(idx, p1) do {               \
    U8  *p = PTR(idx);                     \
    U8  f8 = *(p);                         \
    U16 a  = IDX(p1);                      \
    SET16(p, (a | (U16)f8<<8));            \
    } while(0)
#define JMPBCK(idx, f) SET16(here, (idx) | ((f)<<8))
///@}
///
///@name Stack Ops (note: rp grows downward)
///@{
#define RPUSH(a)       (*(rp++)=(U16)(a))           /**< push address onto return stack */
#define RPOP()         (*(--rp))                    /**< pop address from return stack  */
///@}
///
///@name Dictionary Index <=> Pointer Converter
///@{
#define PTR(n)         ((U8*)dic + (n))             /**< convert dictionary index to a memory pointer */
#define IDX(p)         ((U16)((U8*)(p) - dic))      /**< convert memory pointer to a dictionary index */
///@}
///
///> Assembler Object initializer
///
N4Asm::N4Asm(U8 *mem) : dic(mem)
{
    reset();
}
///
///> reset internal pointers (called by VM::reset)
///
void N4Asm::reset()
{
    here = dic;                       // rewind to dictionary base
    last = PTR(0xffff);               // -1
    tab  = 0;
    
    set_trace(0);
}
///
///> parse given token into actionable item
///
N4OP N4Asm::parse_token(U8 *tkn, U16 *rst, U8 run)
{
    if (query(tkn, rst))                 return TKN_DIC; /// * DIC - is a word in dictionary? [adr(2),name(3)]
    if (find(tkn, run ? CMD : JMP, rst)) return TKN_IMM; /// * IMM - is a immediate word?
    if (find(tkn, PRM, rst))             return TKN_PRM; /// * PRM - is a primitives?
    if (number(tkn, (S16*)rst))          return TKN_NUM; /// * NUM - is a number literal?
    return TKN_ERR;                                      /// * ERR - unknown token
}
///
///> create word onto dictionary
///
void N4Asm::compile(U16 *rp0)
{
    rp = rp0;                       // set return stack pointer
    U8 *l0 = last, *h0 = here;
    U8 *p0 = here;
    U8 trc = is_tracing();

    _do_header();                   /// **fetch token, create name field linked to previous word**

    for (U8 *tkn=p0; tkn;) {        ///> loop til token exausted (tkn==NULL)
        U16 tmp;
        if (trc) d_mem(dic, p0, (U16)(here-p0), 0);  ///>> trace assember progress if enabled

        tkn = token();
        p0  = here;                         // keep current top of dictionary (for memdump)
        switch(parse_token(tkn, &tmp, 0)) { ///>> **determinie type of operation, and keep opcode in tmp**
        case TKN_IMM:                       ///>> an immediate command?
            if (tmp==OP_SEMI) {             /// * handle return i.e. ; (semi-colon)
                SET8(here, PFX_RET);        //  terminate COLON definitions, or
                tkn = NULL;                 //  clear token to exit compile mode
            }
            else _do_branch(tmp);           /// * add branching opcode
            break;
        case TKN_DIC:                       ///>> a dictionary word? [addr + adr(2) + name(3)]
            JMPBCK(tmp+2+3, PFX_CALL);      /// * call subroutine
            break;
        case TKN_PRM:                       ///>> a built-in primitives?
            SET8(here, PFX_PRM | (U8)tmp);  /// * add found primitive opcode
            if (tmp==I_DQ) _do_str();       /// * do extra, if it's a ." (dot_string) command
            break;
        case TKN_NUM:                       ///>> a literal (number)?
            if (tmp < 128) {
                SET8(here, (U8)tmp);        /// * 1-byte literal, or
            }
            else {
                SET8(here, PFX_PRM | I_LIT);/// * 3-byte literal
                SET16(here, tmp);
            }
            break;
        default:                            ///>> then, token type not found
            flash("??  ");
            last = l0;                      /// * restore last, here pointers
            here = h0;
            token(TIB_CLR);
            tkn  = NULL;                    /// * bail, terminate loop!
        }
    }
    ///> debug memory dump
    if (trc && last>l0) d_mem(dic, last, (U16)(here-last), ' ');
}
///
///> create a variable on dictionary
/// * note: 8 or 10-byte per variable
///
void N4Asm::variable()
{
    _do_header();                           /// **fetch token, create name field linked to previous word**

    U8 tmp = IDX(here+2);                   // address to variable storage
    if (tmp < 128) {                        ///> handle 1-byte address + RET(1)
        SET8(here, (U8)tmp);
    }
    else {
        tmp += 2;                           ///> or, extra bytes for 16-bit address
        SET8(here, PFX_PRM | I_LIT);
        SET16(here, tmp);
    }
    SET8(here, PFX_RET);
    SET16(here, 0);                         /// add actual literal storage area
}
///
///> create a constant on dictionary
/// * note: 8 or 10-byte per variable
///
void N4Asm::constant(S16 v)
{
    _do_header();                           /// **fetch token, create name field linked to previous word**

    if (v < 128) {                          ///> handle 1-byte constant
        SET8(here, (U8)v);
    }
    else {
        SET8(here, PFX_PRM | I_LIT);        ///> or, constant stored as 3-byte literal 
        SET16(here, v);
    }
    SET8(here, PFX_RET);
}
///
///> scan the keyword through dictionary linked-list
/// @return
///    1 - token found<br/>
///    0 - token not found
///
U8 N4Asm::query(U8 *tkn, U16 *adr)
{
    for (U8 *p=last; p!=PTR(0xffff); p=PTR(GET16(p))) {
        if (uc(p[2])==uc(tkn[0]) &&
            uc(p[3])==uc(tkn[1]) &&
            (p[3]==' ' || uc(p[4])==uc(tkn[2]))) {
            *adr = IDX(p);
            return 1;
        }
    }
    return 0;
}
///
///> display words in dictionary
///
constexpr U8 WORDS_PER_ROW = 20;        ///< words per row when showing dictionary
void N4Asm::words()
{
    U8 trc = is_tracing();
    U8 n   = 0, wpr = WORDS_PER_ROW >> (trc ? 1 : 0);
    for (U8 *p=last; p!=PTR(0xffff); p=PTR(GET16(p)), n++) {  /// **from last, loop through dictionary**
        if ((n%wpr)==0) d_chr('\n');                          ///>> linefeed for every WORDS_PER_ROW
        if (trc) { d_adr(IDX(p)); d_chr(':'); }               ///>> optionally show address
        d_chr(p[2]); d_chr(p[3]); d_chr(p[4]); d_chr(' ');    ///>> 3-char name + space
    }
    _list_voc();                                              ///> list built-in vocabularies
}
///
///> drop words from the dictionary
///
void N4Asm::forget()
{
    U16 adr;
    if (!query(token(), &adr)) {            /// check if token is in dictionary
        flash("?!  ");                      /// * not found, bail
        return;
    }
    ///
    /// word found, rollback here
    ///
    U8 *p = PTR(adr);                       // address of word
    last  = PTR(GET16(p));                  /// * reset last word address
    here  = p;                              /// * reset current pointer
}
///
///> persist dictionary from RAM into EEPROM
///
constexpr U16 N4_SIG  = (((U16)'N'<<8)+(U16)'4');  ///< EEPROM signature
constexpr U16 ROM_HDR = 6;                         ///< EEPROM header size
void N4Asm::save()
{
    U8  trc    = is_tracing();
    U16 last_i = IDX(last);
    U16 here_i = IDX(here);
    
    if (trc) flash("dic>>ROM ");
#if ARDUINO
    ///
    /// verify EEPROM capacity to hold user dictionary
    ///
    if ((ROM_HDR + here_i) > EEPROM.length()) {
        flash("ERROR: dictionary larger than EEPROM");
        return;
    }
    ///
    /// create EEPROM dictionary header
    ///
    EEPROM.update(0, N4_SIG>>8); EEPROM.update(1, N4_SIG&0xff);
    EEPROM.update(2, last_i>>8); EEPROM.update(3, last_i&0xff);
    EEPROM.update(4, here_i>>8); EEPROM.update(5, here_i&0xff);
    ///
    /// copy user dictionary into EEPROM byte-by-byte
    ///
    U8 *p = dic;
    for (int i=0; i<here_i; i++) {
        EEPROM.update(ROM_HDR+i, *p++);
    }
#endif //ARDUINO
    
    if (trc) {
        d_num(here_i);
        flash(" bytes saved\n");
    }
}
///
///> restore dictionary from EEPROM into RAM
///
void N4Asm::load()
{
    U8 trc = is_tracing();
    
    if (trc) flash("dic<<ROM ");
#if ARDUINO
    ///
    /// validate EEPROM contains user dictionary (from previous run)
    ///
    U16 n4     = ((U16)EEPROM.read(0)<<8) + EEPROM.read(1);
    if (n4 != N4_SIG) return;       // not intialized yet
    ///
    /// retrieve metadata (sizes) of user dicationary
    ///
    U16 last_i = ((U16)EEPROM.read(2)<<8) + EEPROM.read(3);
    U16 here_i = ((U16)EEPROM.read(4)<<8) + EEPROM.read(5);
    ///
    /// retrieve user dictionary byte-by-byte into memory
    ///
    U8 *p = dic;
    for (int i=0; i<here_i; i++) {
        *p++ = EEPROM.read(ROM_HDR+i);
    }
    ///
    /// adjust user dictionary pointers
    ///
    last = PTR(last_i);
    here = PTR(here_i);

    if (trc) {
        d_num(here_i);
        flash(" bytes loaded\n");
    }
#endif //ARDUINO
}
///
///> execution tracer (debugger, can be modified into single-stepper)
///
void N4Asm::trace(U16 a, U8 ir)
{
    if (!is_tracing()) return;                        ///> check tracing flag
    
    d_adr(a);                                         // opcode address

    U8 *p, op = ir & CTL_BITS;
    switch (op) {
    case 0xc0:                                        ///> is a jump instruction?
        a = GET16(PTR(a)) & ADR_MASK;                 // target address
        switch (ir & JMP_MASK) {                      // get branching opcode
        case PFX_CALL:                                // 0xc0 CALL word call
            d_chr(':');
            p = PTR(a)-3;                             // backtrack 3-byte (name field)
            d_chr(*p++); d_chr(*p++); d_chr(*p);
            flash("\n....");
            for (int i=0, n=++tab; i<n; i++) {        // indentation per call-depth
                flash("  ");
            }
            break;
        case PFX_RET:                                 // 0xd0 RET return
            d_chr(';');
            tab -= tab ? 1 : 0;
            break;
        case PFX_CDJ: d_chr('?'); d_adr(a); break;    // 0xe0 CDJ  conditional jump
        case PFX_UDJ: d_chr('j'); d_adr(a); break;    // 0xf0 UDJ  unconditional jump
        }
        break;
    case 0x80:                                        ///> is a primitive?
        op = ir & PRM_MASK;                           // capture primitive opcode
        switch (op) {
        case I_LIT:                                   // 3-byte literal (i.e. 16-bit signed integer)
            d_chr('#');
            p = PTR(a)+1;                             // address to the 16-bit number
            a = GET16(p);                             // fetch the number (reuse a, bad, but to save)
            d_u8(a>>8); d_u8(a&0xff);
            break;
        case I_DQ:                                    // print string
            d_chr('"');
            p = PTR(a)+1;                             // address to string header
            d_str(p);                                 // print the string to console
            break;
        default:                                      // other opcodes
            d_chr('_');
            U8 ci = op >= I_FOR;                      // loop controller flag
            d_name(ci ? op-I_FOR : op, ci ? PMX : PRM, 0);
        }
        break;
    default:                                          ///> and a number (i.e. 1-byte literal)
        d_chr('#'); d_u8(ir);
    }
    d_chr(' ');
}
///
///> create name field with link back to previous word
///
void N4Asm::_do_header()
{
    U8  *tkn = token();             ///#### fetch one token from console
    U16 tmp  = IDX(last);           // link to previous word
    
    last = here;                    ///#### create 3-byte name field
    SET16(here, tmp);               // pointer to previous word
    SET8(here, tkn[0]);             // store token into 3-byte name field
    SET8(here, tkn[1]);
    SET8(here, tkn[1]!=' ' ? tkn[2] : ' ');
}
///
///> create branching for instructions
///>> f IF...THN, f IF...ELS...THN
///>> BGN...f UTL, BGN...f WHL...RPT, BGN...f WHL...f UTL
///>> n1 n0 FOR...NXT
///
void N4Asm::_do_branch(U8 op)
{
    switch (op) {
    case 1: /* IF */
        RPUSH(IDX(here));               // save current here A1
        JMP000(here, PFX_CDJ);          // alloc addr with jmp_flag
        break;
    case 2: /* ELS */            
        JMPSET(RPOP(), here+2);         // update A1 with next addr
        RPUSH(IDX(here));               // save current here A2
        JMP000(here, PFX_UDJ);          // alloc space with jmp_flag
        break;
    case 3: /* THN */
        JMPSET(RPOP(), here);           // update A2 with current addr
        break;
    case 4: /* BGN */
        RPUSH(IDX(here));               // save current here A1
        break;
    case 5: /* UTL */
        JMPBCK(RPOP(), PFX_CDJ);        // conditional jump back to A1
        break;
    case 6: /* WHL */
        RPUSH(IDX(here));               // save WHILE addr A2
        JMP000(here, PFX_CDJ);          // allocate branch addr A2 with jmp flag
        break;
    case 7: /* RPT */
        JMPSET(RPOP(), here+2);         // update A2 with next addr
        JMPBCK(RPOP(), PFX_UDJ);        // unconditional jump back to A1
        break;
    case 8: /* FOR */
        RPUSH(IDX(here+1));             // save current addr A1
        SET8(here, PFX_PRM | I_FOR);    // encode FOR opcode
        break;
    case 9: /* NXT */
        SET8(here, PFX_PRM | I_NXT);    // encode NXT opcode
        JMPBCK(RPOP(), PFX_CDJ);        // conditionally jump back to A1
        SET8(here, PFX_PRM | I_BRK);    // encode BRK opcode
        break;
    case 10: /* I */
        SET8(here, PFX_PRM | I_I);      // fetch loop counter
        break;
    }
}
///
///> display the opcode name
/// 
void N4Asm::_do_str()
{
    U8 *p0 = token();                  // get string from input buffer
    U8 sz  = 0;
    for (U8 *p=p0; *p!='"'; p++, sz++);
    SET8(here, sz);
    for (int i=0; i<sz; i++) SET8(here, *p0++);
}
///
///> list words in built-in vocabularies
///
void N4Asm::_list_voc()
{
    const char *lst[] PROGMEM = { PRM, JMP, CMD };      // list of built-in primitives
    for (U8 i=0, n=0; i<3; i++) {
#if ARDUINO
        U8 sz = pgm_read_byte(reinterpret_cast<PGM_P>(lst[i]));
#else
        U8 sz = *(lst[i]);
#endif //ARDUINO
        while (sz--) {
            d_chr(n++%WORDS_PER_ROW ? ' ' : '\n');
            d_name(sz, lst[i], 1);
        }
    }
    d_chr('\n');
}

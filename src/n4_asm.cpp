/**
 * @file
 * @brief nanoForth Assmebler implementation
 *
 * ####Assembler Memory Map:
 *
 * @code
 *    mem[...dic_sz...[...stk_sz...]
 *       |            |            |
 *       +-dic-->     +-->sp  rp<--+
 * @endcode
 */
#include "n4_asm.h"
#if ARDUINO
#include <EEPROM.h>
#else
#include "mockrom.h"
#endif //ARDUINO

#include "n4_core.h"
using namespace N4Core;                       /// * make utilities available

///
///@name nanoForth built-in vocabularies
///
/// @var CMD
/// @brief words for interpret mode.
/// @var JMP (note: ; is hard-coded at position 0, do not change it)
/// @brief words for branching ops in compile mode.
/// @var PRM
/// @brief primitive words (53 of 61 allocated, 3 pre-allocated).
/// @var PMX
/// @brief loop control opcodes
///
///@{
PROGMEM const char IMM[] = "\xf"                                \
    ":  " "VAR" "VAL" "PCI" "TMI" "HEX" "DEC" "FGT" "WRD" "DMP" \
    "SEE" "SAV" "LD " "SEX" "BYE";
    // TODO: "s\" "
PROGMEM const char JMP[] = "\x0b" \
    ";  " "IF " "ELS" "THN" "BGN" "UTL" "WHL" "RPT" "I  " "FOR" \
    "NXT";

#define N4_WORDS \
    "NOP" "DRP" "DUP" "SWP" "OVR" "ROT" "+  " "-  " "*  " "/  " \
    "MOD" "NEG" "AND" "OR " "XOR" "NOT" "LSH" "RSH" "=  " "<  " \
    ">  " "<> " "@  " "!  " "C@ " "C! " "KEY" "EMT" "CR " ".  " \
    ".\" "">R " "R> " "HRE" "RND" "ALO" "TRC" "CLK" "D+ " "D- " \
    "DNG" "ABS" "MAX" "MIN" "DLY" "IN " "AIN" "OUT" "PWM" "PIN" \
    "TME" "PCE" "API"

PROGMEM const char PRM[] =
#if N4_DOES_META
    "\x3b" N4_WORDS "CRE" ",  " "C, " "'  " "EXE" "DO>";
#else
    "\x35" N4_WORDS;
#endif // N4_DOES_META

PROGMEM const char PMX[] = "\x2" "I  " "FOR";
///@}
///
///@name Branching
///@{
#define JMP00(j)      ENC16(here, (j)<<8)
#define JMPTO(idx, f) ENC16(here, (idx) | ((f)<<8))
#define JMPSET(idx, p1) do {               \
    U8  *p = DIC(idx);                     \
    U8  f8 = *(p);                         \
    U16 a  = IDX(p1);                      \
    ENC16(p, (a | (U16)f8<<8));            \
    } while(0)
///@}
///
///@name Stack Ops (note: return stack grows downward)
///@{
#define RPUSH(a)       (*(vm.rp++)=(U16)(a))       /**< push address onto return stack */
#define RPOP()         (*(--vm.rp))                /**< pop address from return stack  */
///@}
///
///@name Dictionary Index <=> Pointer Converter
///@{
#define DIC(n)         ((U8*)dic + (n))            /**< convert dictionary index to a memory pointer */
#define IDX(p)         ((U16)((U8*)(p) - dic))     /**< convert memory pointer to a dictionary index */
///@}
constexpr U16 N4_SIG  = (((U16)'N'<<8)+(U16)'4');  ///< EEPROM signature
constexpr U16 N4_AUTO = N4_SIG | 0x8080;           ///< EEPROM auto-run signature
constexpr U16 ROM_HDR = 6;                         ///< EEPROM header size
constexpr U8  WORDS_PER_ROW = 16;                  ///< words per row when showing dictionary

namespace N4Asm {

U8  *last  { NULL };                ///< pointer to last word, for debugging
U8  *here  { NULL };                ///< top of dictionary (exposed to _vm for HRE, ALO opcodes)
U8  tab = 0;                        ///< tracing indentation counter
///
///> find colon word address of next input token
/// @brief search the keyword through colon word linked-list
/// @return
///    1 - token found<br/>
///    0 - token not found
///
U8 _find(U8 *tkn, U16 *adr)
{
    for (U8 *p=last, *ex=DIC(LFA_END); p!=ex; p=DIC(GET16(p))) {
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
///> create name field with link back to previous word
///
void _add_word()
{
    U8  *tkn = get_token();         ///#### fetch one token from console
    U16 tmp  = IDX(last);           // link to previous word

    last = here;                    ///#### create 3-byte name field
    ENC16(here, tmp);               // lfa: pointer to previous word
    ENC8(here, tkn[0]);             // nfa: store token into 3-byte name field
    ENC8(here, tkn[1]);
    ENC8(here, tkn[1]!=' ' ? tkn[2] : ' ');
}
///
///> create branching for instructions
///>> f IF...THN, f IF...ELS...THN
///>> BGN...f UTL, BGN...f WHL...RPT, BGN...f WHL...f UTL
///>> n1 n0 FOR...NXT
///
void _add_branch(U8 op)
{
    switch (op) {
    case 0: /* ; */
        ENC8(here, PRM_OPS | I_RET);    // semi colon, mark end of a colon word
        break;
    case 1: /* IF */
        RPUSH(IDX(here));               // save current here A1
        JMP00(OP_CDJ);                  // alloc addr with jmp_flag
        break;
    case 2: /* ELS */
        JMPSET(RPOP(), here+2);         // update A1 with next addr
        RPUSH(IDX(here));               // save current here A2
        JMP00(OP_UDJ);                  // alloc space with jmp_flag
        break;
    case 3: /* THN */
        JMPSET(RPOP(), here);           // update A2 with current addr
        break;
    case 4: /* BGN */
        RPUSH(IDX(here));               // save current here A1
        break;
    case 5: /* UTL */
        JMPTO(RPOP(), OP_CDJ);          // conditional jump back to A1
        break;
    case 6: /* WHL */
        RPUSH(IDX(here));               // save WHILE addr A2
        JMP00(OP_CDJ);                  // allocate branch addr A2 with jmp flag
        break;
    case 7: /* RPT */
        JMPSET(RPOP(), here+2);         // update A2 with next addr
        JMPTO(RPOP(), OP_UDJ);          // unconditional jump back to A1
        break;
    case 8: /* I */
        ENC8(here, PRM_OPS | I_I);      // fetch loop counter
        break;
    case 9: /* FOR */
        RPUSH(IDX(here+1));             // save current addr A1
        ENC8(here, PRM_OPS | I_FOR);    // encode FOR opcode
        break;
    case 10: /* NXT */
        JMPTO(RPOP(), OP_NXT);          // loop back to A1
        break;
    }
}
///
///> display the opcode name
///
void _add_str()
{
    U8 *p0 = get_token();               // get string from input buffer
    U8 sz  = 0;
    for (U8 *p=p0; *p!='"'; p++, sz++);
    ENC8(here, sz);
    for (int i=0; i<sz; i++) ENC8(here, *p0++);
}
///
///> list words in built-in vocabularies
///
void _list_voc(U16 n)
{
    const char *lst[] PROGMEM = { IMM, JMP, PRM };      // list of built-in primitives
    for (U8 i=0; i<3; i++) {
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
}
///
///> persist dictionary from RAM into EEPROM
///
void save(bool autorun)
{
    U16 here_i = IDX(here);

    if (trc) show("dic>>ROM ");

    U16 last_i = IDX(last);
    ///
    /// verify EEPROM capacity to hold user dictionary
    ///
    if ((ROM_HDR + here_i) > EEPROM.length()) {
        show("ERROR: dictionary larger than EEPROM");
        return;
    }
    ///
    /// create EEPROM dictionary header
    ///
    U16 sig = autorun ? N4_AUTO : N4_SIG;
    EEPROM.update(0, sig>>8);    EEPROM.update(1, sig   &0xff);
    EEPROM.update(2, last_i>>8); EEPROM.update(3, last_i&0xff);
    EEPROM.update(4, here_i>>8); EEPROM.update(5, here_i&0xff);
    ///
    /// copy user dictionary into EEPROM byte-by-byte
    ///
    U8 *p = dic;
    for (int i=0; i<here_i; i++) {
        EEPROM.update(ROM_HDR+i, *p++);
    }
    if (trc) {
        d_num(here_i);
        show(" bytes saved\n");
    }
}
///
///> restore dictionary from EEPROM into RAM
/// @return
///    lnk:     autorun address (of last word from EEPROM)
///    LFA_END: no autorun or EEPROM not been setup yet
///
U16 load(bool autorun)
{
    if (trc && !autorun) show("dic<<ROM ");
    ///
    /// validate EEPROM contains user dictionary (from previous run)
    ///
    U16 n4 = ((U16)EEPROM.read(0)<<8) + EEPROM.read(1);
    if (autorun) {
        if (n4 != N4_AUTO) return LFA_END;          // EEPROM is not set to autorun
    }
    else if (n4 != N4_SIG) return LFA_END;          // EEPROM has no saved words
    ///
    /// retrieve metadata (sizes) of user dictionary
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
    last = DIC(last_i);
    here = DIC(here_i);

    if (trc && !autorun) {
        d_num(here_i);
        show(" bytes loaded\n");
    }
    return last_i;
}
///
///> reset internal pointers (called by VM::reset)
/// @return
///  1: autorun last word from EEPROM
///  0: clean start
///
U16 reset()
{
    here    = dic;                       // rewind to dictionary base
    last    = DIC(LFA_END);              // root of linked field
    tab     = 0;
    
#if ARDUINO
    trc = 0;
#else
    trc = 1;                             // tracing on PC
#endif // ARDUINO

    return load(true);
}
///
///> get address of next input token
///
U16 query() {
    U16 adr;                        ///< lfa of word
    if (!_find(get_token(), &adr)) {/// check if token is in dictionary
        show("?!  ");               /// * not found, bail
        return 0;
    }
    return adr + 2 + 3;             /// * xt = adr + lnk[2] + name[3]
}
///
///> parse given token into actionable item
///
N4OP parse(U8 *tkn, U16 *rst, U8 run)
{
    if (_find(tkn, rst))                 return TKN_WRD; /// * WRD - is a colon word? [lnk(2),name(3)]
    if (scan(tkn, run ? IMM : JMP, rst)) return TKN_IMM; /// * IMM - is a immediate word?
    if (scan(tkn, PRM, rst))             return TKN_PRM; /// * PRM - is a primitives?
    if (number(tkn, (S16*)rst))          return TKN_NUM; /// * NUM - is a number literal?
    return TKN_ERR;                                      /// * ERR - unknown token
}
///
///> Forth assembler (creates word onto dictionary)
///
void compile(U16 *rp0)
{
    vm.rp = rp0;                       // set return stack pointer
    U8 *l0 = last, *h0 = here;
    U8 *p0 = here;

    _add_word();                    /// **fetch token, create name field linked to previous word**

    for (U8 *tkn=p0; tkn;) {        ///> loop til exhaust all tokens (tkn==NULL)
        U16 tmp;
        if (trc) d_mem(dic, p0, (U16)(here-p0), 0);  ///>> trace assembler progress if enabled

        tkn = get_token();
        p0  = here;                         // keep current top of dictionary (for memdump)
        switch(parse(tkn, &tmp, 0)) {       ///>> **determine type of operation, and keep opcode in tmp**
        case TKN_IMM:                       ///>> an immediate command?
            _add_branch(tmp);               /// * add branching opcode
            if (tmp==I_RET) {
                tkn = NULL;                 /// * clear token to exit compile mode
                if (trc) d_mem(dic, last, (U16)(here-last), ' ');  ///> debug memory dump, if enabled
            }
            break;
        case TKN_WRD:                       ///>> a colon word? [addr + lnk(2) + name(3)]
            JMPTO(tmp+2+3, OP_CALL);        /// * call subroutine
            break;
        case TKN_PRM:                       ///>> a built-in primitives?
            ENC8(here, PRM_OPS | (U8)tmp);  /// * add found primitive opcode
            if (tmp==I_DQ) _add_str();      /// * do extra, if it's a ." (dot_string) command
            break;
        case TKN_NUM:                       ///>> a literal (number)?
            if (tmp < 128) {
                ENC8(here, (U8)tmp);        /// * 1-byte literal, or
            }
            else {
                ENC8(here, PRM_OPS | I_LIT);/// * 3-byte literal
                ENC16(here, tmp);
            }
            break;
        case TKN_EXT:                       ///>> extended words, not implemented yet
        default:                            ///>> then, token type not found
            show("??  ");
            last = l0;                      /// * restore last, here pointers
            here = h0;
            clear_tib();                    /// * reset tib and token parser
            tkn  = NULL;                    /// * bail, terminate loop!
        }
    }
}
///
///> meta compiler
///
void create() {                             ///> create a word header (link + name field)
    _add_word();                            /// **fetch token, create name field linked to previous word**

    U8 tmp = IDX(here+2);                   // address to variable storage
    if (tmp < 128) {                        ///> handle 1-byte address + RET(1)
        ENC8(here, (U8)tmp);
    }
    else {
        tmp += 2;                           ///> or, extra bytes for 16-bit address
        ENC8(here, PRM_OPS | I_LIT);
        ENC16(here, tmp);
    }
    ENC8(here, PRM_OPS | I_RET);
}
void comma(S16 v)  { ENC16(here, v); }      ///> compile a 16-bit value onto dictionary
void ccomma(S16 v) { ENC8(here, v);  }      ///> compile a 16-bit value onto dictionary
void does(U16 xt)  {                        ///> metaprogrammer (jump to definding word DO> section)
#if N4_DOES_META
	U8 *p = here - 1;                               /// start walking back
    for (; *p!=(PRM_OPS|I_RET); p--) *(p+2) = *p;   /// shift down parameters by 2 bytes
    *(p-1) += 2;                                    /// adjust the PFA
    ENC16(p, xt | (OP_UDJ << 8));                   /// replace RET with a JMP,
	ENC8(p, PRM_OPS|I_RET);                         /// and a RET, (not necessary but nice to SEE)
	here += 2;                                      /// extra 2 bytes due to shift
#endif // N4_DOES_META
}
///
///> create a variable on dictionary
/// * note: 8 or 10-byte per variable
///
void variable()
{
    create();
    ENC16(here, 0);                         /// add actual literal storage area
}
///
///> create a constant on dictionary
/// * note: 8 or 10-byte per variable
///
void constant(S16 v)
{
    _add_word();                            /// **fetch token, create name field linked to previous word**

    if (v < 128) {                          ///> handle 1-byte constant
        ENC8(here, (U8)v);
    }
    else {
        ENC8(here, PRM_OPS | I_LIT);        ///> or, constant stored as 3-byte literal 
        ENC16(here, v);
    }
    ENC8(here, PRM_OPS | I_RET);
}
///
///> display words in dictionary
///
void words()
{
    U8  wrp = WORDS_PER_ROW >> (trc ? 1 : 0);                    ///> wraping width
    U16 n   = 0;
    for (U8 *p=last, *ex=DIC(LFA_END); p!=ex; p=DIC(GET16(p))) { /// **from last, loop through dictionary**
        d_chr(n++%wrp ? ' ' : '\n');
        if (trc) { d_adr(IDX(p)); d_chr(':'); }                  ///>> optionally show address
        d_chr(p[2]); d_chr(p[3]); d_chr(p[4]);                   ///>> 3-char name
    }
    _list_voc(trc ? n<<1 : n);                                   ///> list built-in vocabularies
    d_chr(' ');
}
///
///> drop words from the dictionary
///
void forget()
{
    U16 xt = query();                  ///< cfa of word
    if (!xt) return;                   /// * bail if word not found
    ///
    /// word found, rollback here
    ///
    U8 *lfa = DIC(xt - 2 - 3);         ///< pointer to word's link
    last    = DIC(GET16(lfa));         /// * reset last word address
    here    = lfa;                     /// * reset current pointer
}
///
///> decode colon word
///
void see()
{
    U16 xt = query();                  ///< cfa of word
    if (!xt) return;                   /// * bail if word not found
    ///
    /// word found, walk parameter field
    ///
    d_chr('\n');
    for (U8 ir = *DIC(xt); ir != (PRM_OPS|I_RET); ir = *DIC(xt)) {
        xt = trace(xt, ir, '\n');
    }
    d_adr(xt); show("_; ");
}
///
///> execution tracer (debugger, can be modified into single-stepper)
///
U16 trace(U16 a, U8 ir, char delim)
{
    d_adr(a);                                         // opcode address

    switch (ir & CTL_BITS) {
    case JMP_OPS: {                                   ///> is a jump instruction?
        U16 w = GET16(DIC(a)) & ADR_MASK;             // target address
        switch (ir & JMP_MASK) {                      // get branching opcode
        case OP_CALL: {                               // 0xc0 CALL word call
            U8 *p = DIC(w)-3;                         // backtrack 3-byte (name field)
            d_chr(':');
            d_chr(*p++); d_chr(*p++); d_chr(*p);
            if (!delim) {
	            show("\n....");
    	        for (int i=0, n=++tab; i<n; i++) {    // indentation per call-depth
        	        show("  ");
	            }
            }
        } break;
        case OP_CDJ: d_chr('?'); d_adr(w); break;     // 0xd0 CDJ  conditional jump
        case OP_UDJ: d_chr('j'); d_adr(w); break;     // 0xe0 UDJ  unconditional jump
        case OP_NXT:                                  // 0xf0 NXT
            if (!delim) { d_chr('r'); d_adr(w); }
            else show("_NXT");
            break;
        }
        a+=2;                                         // skip over address
    } break;
    case PRM_OPS: {                                   ///> is a primitive?
    	ir &= PRM_MASK;                               // capture primitive opcode
        switch (ir) {
        case I_RET:
        	d_chr('_'); d_chr(';');
            tab -= tab ? 1 : 0;
            break;
        case I_LIT: {                                 // 3-byte literal (i.e. 16-bit signed integer)
            U8 *p = DIC(a)+1;                         // address to the 16-bit number
            S16 w = GET16(p);                         // fetch the number
            d_chr('#');
            d_num(w);
            a += 2;                                   // skip literal
        } break;
        case I_DQ: {                                  // print string
            U8 *p = DIC(a)+1;                         // address to string header
            d_chr('"');
            d_str(p);                                 // print the string to console
            a += *p;
        } break;
        default:                                      // other opcodes
            d_chr('_');
            U8 ci = ir >= I_I;                        // loop controller flag
            d_name(ci ? ir-I_I : ir, ci ? PMX : PRM, 0);
        }
        a++;
    } break;
    default:                                          ///> and a number (i.e. 1-byte literal)
        d_chr('#'); d_num((S16)ir);
        a++;           
    }
    d_chr(delim ? delim : ' ');
    return a;
}

}  // namespace N4Asm

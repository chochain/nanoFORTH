///
/// \file nanoforth_asm.cpp
/// \brief NanoForth Assmebler implementation
///
///> assembler memory organization:
///>>    `mem[...dic_sz...[...stk_sz...]`<br>
///>>    `   |                         |`<br>
///>>    `  dic-->                 rp<-+`<br>
///

#include "nanoforth_util.h"
#include "nanoforth_asm.h"
#if ARDUINO
#include <EEPROM.h>
#endif //ARDUINO
///
/// NanoForth built-in vocabularies
///
/// \var CMD
/// \brief words for interpret mode.
/// \var JMP (note: ; is hardcoded at position 0, do not change it)
/// \brief words for branching op in compile mode.
/// \var PRM
/// \brief primitive words (45 allocated, 64 max).
/// \var PMX
/// \brief loop control opcodes
///
PROGMEM const char CMD[] = "\x05" \
    ":  " "VAR" "FGT" "DMP" "BYE";
PROGMEM const char JMP[] = "\x0b" \
    ";  " "IF " "ELS" "THN" "BGN" "UTL" "WHL" "RPT" "FOR" "NXT" \
    "I  ";
PROGMEM const char PRM[] = "\x2d" \
    "DRP" "DUP" "SWP" "OVR" "+  " "-  " "*  " "/  " "MOD" "NEG" \
    "AND" "OR " "XOR" "NOT" "=  " "<  " ">  " "<= " ">= " "<> " \
    "@  " "!  " "C@ " "C! " ".  " ".\"" ">R " "R> " "WRD" "HRE" \
    "CEL" "ALO" "SAV" "LD " "TRC" "CLK" "D+ " "D- " "DNG" "DLY" \
    "PIN" "IN " "OUT" "AIN" "PWM";
//
// Forth assembler stack opcode macros (note: rp grows downward)
//
#define RPUSH(a)       (*(rp++)=(U16)(a))           /**< push address onto return stack */
#define RPOP()         (*(--rp))                    /**< pop address from return stack  */
//
// dictionary index <=> pointer translation macros
//
#define PTR(n)         ((U8*)dic + (n))             /**< convert dictionary index to a memory pointer */
#define IDX(p)         ((U16)((U8*)(p) - dic))      /**< convert memory pointer to a dictionary index */
//
// \defSETNM
// \brief name field creation
//
#define SETNM(p, s) do {                   \
    SET8(p, (s)[0]);                       \
    SET8(p, (s)[1]);                       \
    SET8(p, ((s)[1]!=' ') ? (s)[2] : ' '); \
    } while(0)
//
// branching opcodes
//
#define JMP000(p,j) SET16(p, (j)<<8)
#define JMPSET(idx, p1) do {               \
    U8  *p = PTR(idx);                     \
    U8  f8 = *(p);                         \
    U16 a  = IDX(p1);                      \
    SET16(p, (a | (U16)f8<<8));            \
    } while(0)
#define JMPBCK(idx, f) do {                \
    SET16(here, idx | (f<<8));             \
    } while(0)
///
///> NanoForth Assembler initializer
///
N4Asm::N4Asm() {}
void N4Asm::init(U8 *mem)
{
    dic = &mem[0];
    reset();
}
///
///> reset internal pointers
///
void N4Asm::reset()
{
    here = dic;                       // rewind to dictionary base
    last = PTR(0xffff);               // -1
    tab  = 0;
}
///
///> parse given token into actionable item
///
N4OP N4Asm::parse_token(U8 *tkn, U16 *rst, U8 run)
{
    if (query(tkn, rst))                         return TKN_DIC; /// * DIC search word dictionary adr(2),name(3)
    if (N4Util::find(tkn, run ? CMD : JMP, rst)) return TKN_IMM; /// * IMM immediate word
    if (N4Util::find(tkn, PRM, rst))             return TKN_PRM; /// * PRM search primitives
    if (N4Util::getnum(tkn, (S16*)rst))          return TKN_NUM; /// * NUM parse as number literal
    return TKN_ERR;                                              /// * ERR unknown token
}
///
///> NanoForth compiler - create word onto dictionary
///
void N4Asm::compile(U16 *rp0)
{
    rp = rp0;                    // capture current return pointer
    U8  *tkn = N4Util::token();  ///#### fetch one token from console
    U8  *p0  = here;
    U16 tmp  = IDX(last);        // link to previous word

    last = here;                 ///#### create 3-byte name field
    SET16(here, tmp);            // pointer to previous word
    SETNM(here, tkn);            // store token into 3-byte name field

    for (; tkn;) {               // terminate if tkn==NULL
        N4Util::memdump(dic, p0, (U16)(here-p0), 0);

        tkn = N4Util::token();
        p0  = here;                         // keep current top of dictionary (for memdump)
        switch(parse_token(tkn, &tmp, 0)) { ///#### determinie type of operation, and keep opcode in tmp
        case TKN_IMM:                       ///> immediate command
            if (tmp==0) {                   // handle ;
                SET8(here, PFX_RET);        /// * terminate COLON definitions, or
                tkn = NULL;                 //    clear token to exit compile mode
            }
            else _do_branch(tmp);           /// * add branching opcode
            break;
        case TKN_DIC:                       ///> add found word: addr + adr(2) + name(3)
            JMPBCK(tmp+2+3, PFX_CALL);
            break;
        case TKN_PRM:                       ///> built-in primitives
        	SET8(here, PFX_PRM | (U8)tmp);  /// * add found primitive opcode
        	if (tmp==I_DQ) _do_str();  	    // handle ."
            break;
        case TKN_NUM:                       ///> literal (number)
            if (tmp < 128) {
                SET8(here, (U8)tmp);        /// * 1-byte literal, or
            }
            else {
                SET8(here, PFX_PRM | I_LIT);/// * 3-byte literal
                SET16(here, tmp);
            }
            break;
        default:  putstr("!\n");            ///> token type not found, bail!
        }
    }
    // debug memory dump
    N4Util::memdump(dic, last, (U16)(here-last), ' ');
}
///
///> create variable on dictionary
///  * note: 9 or 11-byte per variable
///
void N4Asm::variable()
{
    U8 *tkn = N4Util::token();              // get token
    U16 tmp = IDX(last);                    // index to last word
    
    last = here;
    SET16(here, tmp);                       // link addr of previous word
    SETNM(here, tkn);                       // store token into 3-byte variable name field

    tmp = IDX(here+2);                      // address to variable storage
    if (tmp < 128) {                        // 1-byte address + RET(1)
        SET8(here, (U8)tmp);
    }
	else {
        tmp += 2;                           // extra bytes for 16-bit address
        SET8(here, I_LIT);
        SET16(here, tmp);
    }
    SET8(here, PFX_RET);
    SET16(here, 0);	                        // actual storage area
}
///
///> display words in dictionary
///
void N4Asm::words()
{
    U8 n = 0;
    for (U8 *p=last; p!=PTR(0xffff); p=PTR(GET16(p)), n++) {
        if (n%10==0) D_CHR('\n');
#if EXE_TRACE
        D_ADR(IDX(p)); D_CHR(':');                            // optionally show address
#endif /// EXE_TRACE
        D_CHR(p[2]); D_CHR(p[3]); D_CHR(p[4]); D_CHR(' ');    // 3-char name + space
    }
    _list_voc();
}
///
///> scan the keyword through dictionary linked-list
///
U8 N4Asm::query(U8 *tkn, U16 *adr)
{
    for (U8 *p=last; p!=PTR(0xffff); p=PTR(GET16(p))) {
        if (p[2]==tkn[0] && p[3]==tkn[1] && (p[3]==' ' || p[4]==tkn[2])) {
            *adr = IDX(p);
            return 1;
        }
    }
    return 0;
}
///
///> drop words from the dictionary
///
void N4Asm::forget()
{
    U16 adr;
    if (!query(N4Util::token(), &adr)) {    // query token in dictionary
        putstr("??");                       // not found, bail
        return;
    }
    //
    // word found, rollback here
    //
    U8 *p = PTR(adr);                       // address of word
    last  = PTR(GET16(p));                  // reset last word address
    here  = p;                              // reset current pointer
}
///
///> persist dictionary from RAM into EEPROM
///
void N4Asm::save()
{
#if ARDUINO
    U16 last_i = IDX(last);
    U16 here_i = IDX(here);
    
    EEPROM.update(0, last_i>>8); EEPROM.update(1, last_i&0xff);
    EEPROM.update(2, here_i>>8); EEPROM.update(3, here_i&0xff);
    for (int i=0; i<here_i; i++) {
        EEPROM.update(i+4, *dic++);
    }
#endif //ARDUINO
}
///
///> restore dictionary from EEPROM into RAM
///
void N4Asm::load()
{
#if ARDUINO
    U16 last_i = ((U16)EEPROM.read(0)<<8) + EEPROM.read(1);
    U16 here_i = ((U16)EEPROM.read(2)<<8) + EEPROM.read(3);
    for (int i=0; i<here_i; i++) {
        *dic++ = EEPROM.read(i+4);
    }
    last = PTR(last_i);
    here = PTR(here_i);
#endif //ARDUINO
}
///
///> NanoForth execution tracer (debugger, can be modified into single-stepper)
///
PROGMEM const char PMX[] = " FORNXTBRKI  ";
void N4Asm::trace(U16 a, U8 ir)
{
    D_ADR(a);                                         // opcode address

    U8 *p, op = ir & CTL_BITS;
    switch (op) {
    case 0xc0:                                        ///> a jump instruction
        a = GET16(PTR(a)) & ADR_MASK;                 // target address
        switch (ir & JMP_MASK) {					  // get branching opcode
        case PFX_UDJ: D_CHR('j'); D_ADR(a); break;    // 0x40 UDJ  unconditional jump
        case PFX_CDJ: D_CHR('?'); D_ADR(a); break;    // 0x50 CDJ  conditional jump
        case PFX_CALL:                                // 0x60 CALL word call
            D_CHR(':');
            p = PTR(a)-3;                             // backtrack 3-byte (name field)
            D_CHR(*p++); D_CHR(*p++); D_CHR(*p);
            putstr("\n....");
            for (int i=0, n=++tab; i<n; i++) {        // indentation per call-depth
                putstr("  ");
            }
            break;
        case PFX_RET:                                 // 0x70 RET return
            D_CHR(';');
            tab -= tab ? 1 : 0;
            break;
        }
        break;
    case 0x80:                                        ///> a primitive
        op = ir & PRM_MASK;                           // capture primitive opcode
        if (op==I_LIT) {                              // 3-byte literal (i.e. 16-bit signed integer)
        	D_CHR('#');
            p = PTR(a)+1;                             // address to the 16-bit number
            a = GET16(p);                             // fetch the number (reuse a, bad, but to save)
        	D_HEX(a>>8); D_HEX(a&0xff);
        }
        else {                                        // other opcodes
            D_CHR('_');
            U8 ci = op >= I_FOR;                      // loop controller flag
            _opname(ci ? op-I_FOR : op, ci ? PMX : PRM, 0);
        }
        break;
    default:
        D_CHR('#'); D_HEX(ir);                        ///> a number (i.e. 1-byte literal)
    }

    D_CHR(' ');
}
///
///> list words in built-in vocabularies
///
void N4Asm::_list_voc()
{
#define WORDS_PER_ROW 10
    PROGMEM const char *lst[] = { CMD, JMP, PRM };      // list of built-in primitives
    for (U8 i=0, n=0; i<4; i++) {
#if ARDUINO
        U8 sz = pgm_read_byte(reinterpret_cast<PGM_P>(lst[i]));
#else
        U8 sz = *(lst[i]);
#endif //ARDUINO
        for (U8 op=0; op<sz; op++) {
            D_CHR(n++%WORDS_PER_ROW==0 ? '\n' : ' ');
            _opname(op, lst[i], 1);
        }
    }
    D_CHR('\n');
}
///
///> create branching for instructions
///
///>> f IF...THN, f IF...ELS...THN
///>> BGN...RPT, BGN...f UTL, BGN...f WHL...RPT, BGN...f WHL...f UTL
///>> n1 n0 FOR...NXT
///
void N4Asm::_do_branch(U8 op)
{
    switch (op) {
    case 1:	/* IF */
        RPUSH(IDX(here));               // save current here A1
        JMP000(here, PFX_CDJ);          // alloc addr with jmp_flag
        break;
    case 2:	/* ELS */            
        JMPSET(RPOP(), here+2);         // update A1 with next addr
        RPUSH(IDX(here));               // save current here A2
        JMP000(here, PFX_UDJ);          // alloc space with jmp_flag
        break;
    case 3:	/* THN */
        JMPSET(RPOP(), here);           // update A2 with current addr
        break;
    case 4:	/* BGN */
        RPUSH(IDX(here));               // save current here A1
        break;
    case 5:	/* UTL */
        JMPBCK(RPOP(), PFX_CDJ);        // conditional jump back to A1
        break;
    case 6:	/* WHL */
        RPUSH(IDX(here));               // save WHILE addr A2
        JMP000(here, PFX_CDJ);          // allocate branch addr A2 with jmp flag
        break;
    case 7:	/* RPT */
        JMPSET(RPOP(), here+2);         // update A2 with next addr
        JMPBCK(RPOP(), PFX_UDJ);        // unconditional jump back to A1
        break;
    case 8:	/* FOR */
        RPUSH(IDX(here+1));             // save current addr A1
        SET8(here, PFX_PRM | I_FOR);    // encode FOR opcode
        break;
    case 9:	/* NXT */
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
void N4Asm::_opname(U8 op, const char *lst, U8 space)
{
#if ARDUINO
    PGM_P p = reinterpret_cast<PGM_P>(lst)+1+op*3;
#else
    U8 *p = (U8*)lst+1+op*3;
#endif //ARDUINO
    char  c;
    D_CHR(pgm_read_byte(p));
    if ((c=pgm_read_byte(p+1))!=' ' || space) D_CHR(c);
    if ((c=pgm_read_byte(p+2))!=' ' || space) D_CHR(c);
}
///
///> display the opcode name
/// 
void N4Asm::_do_str()
{
    U8 *p0 = N4Util::token();            // get string from input buffer
    U8 sz  = 0;
    for (U8 *p=p0; *p!='"'; p++, sz++);
    SET8(here, sz);
    for (int i=0; i<sz; i++) SET8(here, *p0++);
}

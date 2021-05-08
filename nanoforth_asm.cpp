//
// NanoForth Assmebler
//
#include <EEPROM.h>
#include "nanoforth_util.h"
#include "nanoforth_asm.h"
//
// NanoForth built-in vocabularies
//
const char CMD[] PROGMEM = "\x05" \
    ":  " "VAR" "FGT" "DMP" "BYE";
const char JMP[] PROGMEM = "\x0b" \
    ";  " "IF " "ELS" "THN" "BGN" "UTL" "WHL" "RPT" "DO " "LOP" \
    "I  ";
const char PRM[] PROGMEM = "\x19" \
    "DRP" "DUP" "SWP" ">R " "R> " "+  " "-  " "*  " "/  " "MOD" \
    "AND" "OR " "XOR" "=  " "<  " ">  " "<= " ">= " "<> " "NOT" \
    "@  " "!  " "C@ " "C! " ".  ";
const char EXT[] PROGMEM = "\x0e" \
    "HRE" "CP " "OVR" "INV" "CEL" "ALO" "WRD" "SAV" "LD " "DLY" \
    "IN " "OUT" "AIN" "TRC";
//
// Forth assembler stack opcode macros (note: rp grows downward)
//
// mem[...dic_sz...[...stk_sz...]
//    |                         |
//    dic-->                rp<-+
//
#define RPUSH(v)       (*(rp++)=(U16)(v))
#define RPOP()         (*(--rp))
//
// dictionary index <=> pointer translation macros
//
#define PTR(n)         ((U8*)dic + (n))
#define IDX(p)         ((U16)((U8*)(p) - dic))
//
// name creation macro
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
    U16 a  = ((U8*)(p1) - p) + JMP_BIT;    \
    SET16(p, (a | (U16)f8<<8));            \
    } while(0)
#define JMPBCK(idx, f) do {                \
    U8  *p = PTR(idx);                     \
    U16 a  = (U16)(p - here) + JMP_BIT;    \
    SET16(here, a | (f<<8));               \
    } while(0)
//
// NanoForth Assembler initializer
//
N4Asm::N4Asm() {}
void N4Asm::init(U8 *mem)
{
    dic = &mem[0];
    reset();
}
//
// reset internal pointers
//
void N4Asm::reset()
{
    here = dic;                       // rewind to dictionary base
    last = PTR(0xffff);               // -1
    tab  = 0;
}
//
// parse given token into actionable item
//
U8 N4Asm::parse_token(U8 *tkn, U16 *rst, U8 run)
{
    if (query(tkn, rst))                         return TKN_DIC; // search word dictionary addr(2), name(3)
    if (N4Util::find(tkn, run ? CMD : JMP, rst)) return TKN_EXE; // run, compile mode
    if (N4Util::find(tkn, EXT, rst))             return TKN_EXT; // search extended words
    if (N4Util::find(tkn, PRM, rst))             return TKN_PRM; // search primitives
    if (N4Util::getnum(tkn, (S16*)rst))          return TKN_NUM; // parse as number literal
    
    return TKN_ERR;
}
//
// NanoForth compiler - create word onto dictionary
//
void N4Asm::compile(U16 *rp0)
{
    rp = rp0;                    // capture current return pointer
    U8  *tkn = N4Util::token();  // fetch one token from console
    U8  *p0  = here;
    U16 tmp  = IDX(last);        // link to previous word

    last = here;
    SET16(here, tmp);            // pointer to previous word
    SETNM(here, tkn);            // 3-byte name

    for (; tkn;) {               // terminate if tkn==NULL
        N4Util::memdump(dic, p0, (U16)(here-p0), 0);

        tkn = N4Util::token();
        p0  = here;
        switch(parse_token(tkn, &tmp, 0)) {
        case TKN_EXE:
            if (tmp==0) {
                SET8(here, I_RET);              // terminate COLON definitions
                tkn = NULL;
            }
            else _do_branch(tmp);               // add branching opcode
            break;
        case TKN_DIC:                           // add found word: addr + adr(2) + name(3)
            JMPBCK(tmp+2+3, PFX_CALL);
            break;
        case TKN_EXT:                           // add extended primitive word
            SET8(here, I_EXT);
            SET8(here, (U8)tmp);                // extra 256 words available
            break;
        case TKN_PRM:
            SET8(here, PFX_PRM | (U8)tmp);      // add found primitive opcode
            break;
        case TKN_NUM:
            if (tmp < 128) {
                SET8(here, (U8)tmp);            // 1-byte literal
            }
            else {
                SET8(here, I_LIT);              // 3-byte literal
                SET16(here, tmp);
            }
            break;
        default:  putstr("!\n");                // error
        }
    }
    // debug memory dump
    N4Util::memdump(dic, last, (U16)(here-last), ' ');
}
//
//  create variable on dictionary
//
void N4Asm::variable()
{
    U8 *tkn = N4Util::token(); // get token
    U16 tmp = IDX(last);
    
    last = here;
    SET16(here, tmp);          // link addr of previous word
    SETNM(here, tkn);          // 3-byte variable name

    tmp = IDX(here + 2);       // next addr
    if (tmp < 128) {           // 1-byte immediate
        SET8(here, (U8)tmp);
    }
	else {
        tmp = IDX(here + 4);   // alloc LIT(1)+storage_addr(2)+RET(1)
        SET8(here, I_LIT);
        SET16(here, tmp);
    }
    SET8(here, I_RET);
    SET16(here, 0);	           // actual storage area
}
//
// display words in dictionary
//
void N4Asm::words()
{
    U8 n = 0;
    for (U8 *p=last; p!=PTR(0xffff); p=PTR(GET16(p)), n++) {
        if (n%10==0) D_CHR('\n');
#if EXE_TRACE
        D_ADR(IDX(p)); D_CHR(':');                            // optionally show address
#endif // EXE_TRACE
        D_CHR(p[2]); D_CHR(p[3]); D_CHR(p[4]); D_CHR(' ');    // 3-char name + space
    }
    _list_voc();
}
//
// scan the keyword through dictionary linked-list
//
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
//
//  drop words from the dictionary
//
void N4Asm::forget()
{
    U16 adr;
    if (!query(N4Util::token(), &adr)) { // query token in dictionary
        putstr("??");                    // not found, bail
        return;
    }
    //
    // word found, rollback here
    //
    U8 *p = PTR(adr);                    // address of word
    last  = PTR(GET16(p));               // reset last word address
    here  = p;                           // reset current pointer
}
//
// persist dictionary from RAM into EEPROM
//
void N4Asm::save()
{
    U16 last_i = IDX(last);
    U16 here_i = IDX(here);
    
    EEPROM.update(0, last_i>>8); EEPROM.update(1, last_i&0xff);
    EEPROM.update(2, here_i>>8); EEPROM.update(3, here_i&0xff);
    for (int i=0; i<here_i; i++) {
        EEPROM.update(i+4, *dic++);
    }
}
//
// restore dictionary from EEPROM into RAM
//
void N4Asm::load()
{
    U16 last_i = ((U16)EEPROM.read(0)<<8) + EEPROM.read(1);
    U16 here_i = ((U16)EEPROM.read(2)<<8) + EEPROM.read(3);
    for (int i=0; i<here_i; i++) {
        *dic++ = EEPROM.read(i+4);
    }
    last = PTR(last_i);
    here = PTR(here_i);
}
//
// execution tracer
//
const char PMX[] PROGMEM = " LOPI  RD2DO ";
void N4Asm::trace(U16 a, U8 ir, U8 *pc)
{
    D_ADR(a);                                                // opcode address
    
    if ((ir & 0x80)==0) { D_CHR('#'); D_HEX(ir);                   } // 1-byte literal
    else if (ir==I_LIT) { D_CHR('#'); N4Util::putnum(GET16(pc));   } // 3-byte literal
    else if (ir==I_RET) { D_CHR(';'); tab -= tab ? 1 : 0;          } // RET
    else if (ir==I_EXT) { D_CHR('_'); _opname(*pc, EXT, 0);        } // EXT extended words
    else {
        U8 op = ir & 0x1f;                            // opcode or top 5-bit of offset
        a += ((U16)op<<8) + *pc - JMP_BIT;            // JMP_BIT ensure 2's complement (for backward jump)
        switch (ir & 0xe0) {
        case PFX_UDJ: D_CHR('j'); D_ADR(a); break;    // 0x80 unconditional jump
        case PFX_CDJ: D_CHR('?'); D_ADR(a); break;    // 0xa0 conditional jump
        case PFX_CALL:                                // 0xc0 word call
            D_CHR(':');
            pc = PTR(a)-3;                            // backtrack 3-byte (name field)
            D_CHR(*pc++); D_CHR(*pc++); D_CHR(*pc);
            putstr("\n....");
            for (int i=0, n=++tab; i<n; i++) {        // indentation per call-depth
                putstr("  ");
            }
            break;
        case PFX_PRM:
            D_CHR('_');
            _opname(op<25 ? op : op-25, op<25 ? PRM : PMX, 0);
            break;
        }
    }
    D_CHR(' ');
}
//
// list words in built-in vocabularies
//
void N4Asm::_list_voc()
{
    const char *lst[] PROGMEM = { CMD, JMP, PRM, EXT };
    U8 n = 0;
    for (U8 i=0; i<4; i++) {
        U8 sz = pgm_read_byte(reinterpret_cast<PGM_P>(lst[i]));
        for (U8 op=0; op<sz; op++) {
            D_CHR(n++%10==0 ? '\n' : ' ');
            _opname(op, lst[i], 1);
        }
    }
    D_CHR('\n');
}
//
// branching instructions
//
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
    case 8:	/* DO */
        RPUSH(IDX(here+1));             // save current addr A1
        SET8(here, I_P2R2);
        break;
    case 9:	/* LOP */
        SET8(here, I_LOOP);
        JMPBCK(RPOP(), PFX_CDJ);        // conditionally jump back to A1
        SET8(here, I_RD2);
        break;
    case 10: /* I */
        SET8(here, I_I);
        break;
    }
}
//
// display an opcode name
// 
void N4Asm::_opname(U8 op, const char *lst, U8 space)
{
    PGM_P p = reinterpret_cast<PGM_P>(lst)+1+op*3;
    char  c;
    D_CHR(pgm_read_byte(p));
    if ((c=pgm_read_byte(p+1))!=' ' || space) D_CHR(c);
    if ((c=pgm_read_byte(p+2))!=' ' || space) D_CHR(c);
}


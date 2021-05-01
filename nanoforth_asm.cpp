//
// Forth Compiler
//
#include "nanoforth.h"
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

void _do_branch(U8 op)
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
//  create variable on dictionary
//
void variable(void)
{
    U8 *tkn = gettkn();        // get token
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
// create word on dictionary
//
void compile(void)
{
    U8  *tkn = gettkn();
    U8  *p0  = here;
    U16 tmp  = IDX(last);     // link to previous word

    last = here;
    SET16(here, tmp);         // pointer to previous word
    SETNM(here, tkn);         // 3-byte name

    for (; tkn;) {            // terminate if tkn==NULL
        dump(p0, here, 0);

        tkn = gettkn();
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
        default:  putmsg(F("!\n"));             // error
        }
    }
    // debug memory dump
    dump(last, here, ' ');
}

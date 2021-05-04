//
// Forth VM core
//
#include "nanoforth.h"
//
// allocate, initialize dictionary pointers
//
U8   _mem[MEM_SZ];    // default 512
U8   *dic = &_mem[0]; // dictionary space
U8   *here;           // top of dictionary pointer
U8   *last;           // pointer to last word
U16  *rp;             // return stack pointer
S16  *sp;             // parameter stack pointer
//
//  Forget Words in the Dictionary
//
void _forget(void)
{
    U16 adr;
    if (!query(token(), &adr)) { // query token in dictionary
        putstr("??");            // not found, bail
        return;
    }
    //
    // word found, rollback here
    //
    U8 *p = PTR(adr);           // address of word
    last  = PTR(GET16(p));      // reset last word address
    here  = p;                  // reset current pointer
}
//
//  Execute a Primitive Instruction
//
void _primitive(U8 op)
{
    switch (op) {
    case 0:  POP();                      break; // DRP
    case 1:  PUSH(TOS);                  break; // DUP
    case 2:  {                                  // SWP
        U16 x = TOS1;
        TOS1  = TOS;
        TOS   = x;
    } break;
    case 3:  RPUSH(POP());               break; // >R
    case 4:  PUSH(RPOP());               break; // R>
    case 5:	 TOS += POP();               break; // +
    case 6:	 TOS -= POP();               break; // -
    case 7:	 TOS *= POP();               break; // *
    case 8:	 TOS /= POP();               break; // /
    case 9:	 TOS %= POP();               break; // MOD
    case 10: TOS &= POP();               break;	// AND
    case 11: TOS |= POP();               break;	// OR
    case 12: TOS ^= POP();               break; // XOR
    case 13: TOS = POP()==TOS;           break; // =
    case 14: TOS = POP()> TOS;           break; // <
    case 15: TOS = POP()< TOS;           break; // >
    case 16: TOS = POP()>=TOS;           break; // <=
    case 17: TOS = POP()<=TOS;           break; // >=
    case 18: TOS = POP()!=TOS;           break; // <>
    case 19: TOS = (TOS==0);             break;	// NOT
    case 20: { U8 *p = PTR(POP()); PUSH(GET16(p));  } break; // @
    case 21: { U8 *p = PTR(POP()); SET16(p, POP()); } break; // !
    case 22: { U8 *p = PTR(POP()); PUSH((U16)*p);   } break; // C@
    case 23: { U8 *p = PTR(POP()); *p = (U8)POP();  } break; // C!
    case 24: putnum(POP()); putchr(' '); break; // .
    case 25: {	                                // LOOP
        (*(rp-2))++;                      // counter+1
        PUSH(*(rp-2) >= *(rp-1));         // range check
        d_chr('\n');                      // debug info
    } break;
    case 26: RPOP(); RPOP();             break; // RD2
    case 27: PUSH(*(rp-2));              break; // I
    case 28: RPUSH(POP()); RPUSH(POP()); break; // P2R2
    // the following 3 opcodes change pc, done at one level up
    case 29: /* used by I_RET */         break;
    case 30: /* used by I_LIT */         break;
    case 31: /* used by I_EXT */         break;
    }
}

void _ok()
{
    S16 *s0 = (S16*)&_mem[MEM_SZ];      // top of heap
    if (sp > s0) {                      // check stack overflow
        putstr("OVF!\n");
        sp = s0;                        // reset to top of stack block
    }
    for (S16 *p=s0-1; p >= sp; p--) {
        putnum(*p); putchr('_'); 
    }
    putstr("ok ");
}
//
// Execution tracer
//
void _trace(U16 a, U8 ir, U8 *pc)
{
#if EXE_TRACE
    d_adr(a); d_hex(ir); d_chr(' ');                      // tracing info
#endif // EXE_TRACE
}
//
//  Virtual Code Execution
//
void _execute(U16 adr)
{
    RPUSH(0xffff);                                        // safe gaurd return stack
    for (U8 *pc=PTR(adr); pc!=PTR(0xffff); ) {
        U16 a  = IDX(pc);                                 // current program counter
        U8  ir = *(pc++);                                 // fetch instruction

        _trace(a, ir, pc);                                // execution tracing when enabled
        
        if ((ir & 0x80)==0) { PUSH(ir);               }   // 1-byte literal
        else if (ir==I_LIT) { PUSH(GET16(pc)); pc+=2; }   // 3-byte literal
        else if (ir==I_RET) { pc = PTR(RPOP());       }   // RET
        else if (ir==I_EXT) { extended(*pc++);        }   // EXT extended words
        else {
            U8 op = ir & 0x1f;                            // opcode or top 5-bit of offset
            a += ((U16)op<<8) + *pc - JMP_BIT;            // JMP_BIT ensure 2's complement (for backward jump)
            switch (ir & 0xe0) {
            case PFX_UDJ:                                 // 0x80 unconditional jump
                pc = PTR(a);                              // set jump target
                d_chr('\n');                              // debug info
                break;
            case PFX_CDJ:                                 // 0xa0 conditional jump
                pc = POP() ? pc+1 : PTR(a);               // next or target
                break;
            case PFX_CALL:                                // 0xd0 word call
                RPUSH(IDX(pc+1));                         // keep next as return address
                pc = PTR(a);
                break;
            case PFX_PRM:                                 // 0xe0 primitive
                _primitive(op);                           // call primitve function with opcode
            }
        }
    }
}

void vm_setup() {
    rp   = (U16*)&_mem[MEM_SZ - STK_SZ];                  // return stack pointer, grow upward
    sp   = (S16*)&_mem[MEM_SZ];                           // parameter stack pointer, grows downward
    here = dic;                                           // dictionary pointer
    last = PTR(0xffff);                                   // dictionary terminator mark

    putstr("\nnanoFORTH v1.0");
}

void vm_core() {
    U8  *tkn = token();                                   // get a token from console
    U16 tmp;
    switch (parse_token(tkn, &tmp, 1)) {
    case TKN_EXE:
        switch (tmp) {
        case 0:	compile();               break; // : (COLON), create word
        case 1:	variable();              break; // VAR, create variable    
        case 2:	_forget();               break; // FGT
        case 3: dump(POP(), POP());      break; // DMP
        case 4: vm_setup();              break; // BYE
        } break;
    case TKN_DIC: _execute(tmp + 2 + 3); break;
    case TKN_EXT: extended((U8)tmp);     break;
    case TKN_PRM: _primitive((U8)tmp);   break;
    case TKN_NUM: PUSH(tmp);             break;
    default:
        putstr("?\n");
    }
    _ok();  // stack check and prompt OK
}

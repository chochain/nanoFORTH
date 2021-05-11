///
/// \file nanoforth_vm.cpp
/// \brief NanoForth Virtual Machine class implementation
///
///> Forth VM stack opcode macros (notes: rp grows upward and may collide with sp)<br>
///
///>>    `                              SP0 (sp max to protect overwritten of vm object)`<br>
///>>    ` mem[...dic_sz...[...stk_sz...]`<br>
///>>    `    |            |            |`<br>
///>>    `    dic-->       +->rp    sp<-+`<br>
///>>    `                      TOS TOS1 (top of stack)`<br>
///
#include "nanoforth_util.h"
#include "nanoforth_asm.h"
#include "nanoforth_vm.h"
//
// parameter, return stack ops
//
#define SP0            ((S16*)(dic+msz))            /**< base of parameter stack             */
#define TOS            (*sp)                        /**< pointer to top of current stack     */
#define TOS1           (*(sp+1))                    /**< pointer to the second item on stack */
#define PUSH(v)        (*(--sp)=(S16)(v))           /**< push v onto parameter stack         */
#define POP()          (sp<SP0 ? *sp++ : 0)         /**< pop value off parameter stack       */
#define RPUSH(a)       (*(rp++)=(U16)(a))           /**< push address onto return stack      */
#define RPOP()         (*(--rp))                    /**< pop address from return stack       */
//
// dictionary index <=> pointer translation macros
//
#define PTR(n)         ((U8*)dic + (n))             /**< convert dictionary index to a memory pointer */
#define IDX(p)         ((U16)((U8*)(p) - dic))      /**< convert memory pointer to a dictionary index */
///
/// * constructor and initializer
///
N4VM::N4VM() {}
void N4VM::init(U8 *mem, U16 mem_sz, U16 stk_sz)
{
    n4asm = new N4Asm();     // create and initialze assembler
    n4asm->init(mem);
    
    dic = &mem[0];
    msz = mem_sz;
    ssz = stk_sz;
    
    _init();
}
///
/// * show system info
///
void N4VM::info()
{
    char tmp;
    putstr("MEM=x");       puthex(msz);
    putstr("[DIC=x");      puthex(msz - ssz);
    putstr(", STK=x");     puthex(ssz);
    U16 free = IDX(&tmp) - IDX(sp);
#if EXE_TRACE
    putstr("] dic[");      N4Util::d_ptr(dic);
    putstr("..|rp=");      N4Util::d_ptr((U8*)rp);
    putstr(",");           N4Util::d_ptr((U8*)sp);
    putstr("=sp] free=x"); puthex(free);
    putstr(" [");          N4Util::d_ptr((U8*)&tmp);
#else
    putstr("] Free=x");    puthex(free);
#endif /// EXE_TRACE
    putstr(" ");
}
///
/// * virtual machine execute single step
///
void N4VM::step() {
    _ok();                                      // stack check and prompt OK
    
    U8  *tkn = N4Util::token();                 // get a token from console
    U16 tmp;
    switch (n4asm->parse_token(tkn, &tmp, 1)) { ///> parse action from token (keep opcode in tmp)
    case TKN_IMM:                               ///>> immediate words
        switch (tmp) {
        case 0:	n4asm->compile(rp); break;      ///>>> : (COLON), switch into compile mode (for new word)
        case 1:	n4asm->variable();  break;      ///>>> VAR, create new variable
        case 2:	n4asm->forget();    break;      ///>>> FGT, rollback word created
        case 3: N4Util::dump(                   ///>>> DMP, memory dump
            dic,
            PTR(POP()&0xfff0),
            POP()&0xfff0);          break;      
        case 4: _init();            break;      ///>>> BYE, restart the virtual machine
        }                                break;
    case TKN_DIC: _execute(tmp + 2 + 3); break; ///>> execute word from dictionary (user defined)
    case TKN_EXT: _extended((U8)tmp);    break; ///>> execute word in extended built-in list
    case TKN_PRM: _primitive((U8)tmp);   break; ///>> execute primitive built-in word
    case TKN_NUM: PUSH(tmp);             break; ///>> push a number (literal) to stack top
    default:                                    ///>> or, error (unknow action)
        putstr("?\n");
    }
}
///
/// * enable/disable execution tracing
///
void N4VM::set_trace(U16 f)
{
    trc += f ? 1 : (trc ? -1 : 0);
}
//
// reset virtual machine
//
void N4VM::_init() {
    //
    // reset stack pointers and tracing flags
    //
    rp  = (U16*)&dic[msz - ssz];         // return stack pointer, grow upward
    sp  = (S16*)&dic[msz];               // parameter stack pointer, grows downward
    trc = 0;

    n4asm->reset();                      // reset assember

    putstr("\nnanoFORTH v1.0 ");
}
//
// console prompt with stack dump
//
void N4VM::_ok()
{
    S16 *s0 = (S16*)&dic[msz];          // top of heap
    if (sp > s0) {                      // check stack overflow
        putstr("OVF!\n");
        sp = s0;                        // reset to top of stack block
    }
    for (S16 *p=s0-1; p >= sp; p--) {
        N4Util::putnum(*p); putchr('_'); 
    }
    putstr("ok ");
}
//
// opcode execution unit
//
void N4VM::_execute(U16 adr)
{
    RPUSH(0xffff);                                        // safe gaurd return stack
    for (U8 *pc=PTR(adr); pc!=PTR(0xffff); ) {
        U16 a  = IDX(pc);                                 // current program counter
        U8  ir = *(pc++);                                 // fetch instruction

        if (trc) n4asm->trace(a, ir, pc);                 // executioU8n tracing when enabled
        
        if ((ir & 0x80)==0) { PUSH(ir);               }   // 1-byte literal
        else if (ir==I_LIT) { PUSH(GET16(pc)); pc+=2; }   // 3-byte literal
        else if (ir==I_RET) { pc = PTR(RPOP());       }   // RET pop address from return stack
        else if (ir==I_EXT) {                             // EXT extended words
            U8 op = *pc++;                                // fetch opcode
            if (op==0) {                                  // handle ."
                for (U8 i=0, sz=*pc++; i<sz; i++) D_CHR(*pc++);
            }
            else _extended(op);                           // handle other opcodes
        }
        else {
            U8 op = ir & 0x1f;                            // opcode or top 5-bit of offset
            a += ((U16)op<<8) + *pc - JMP_BIT;            // JMP_BIT ensure 2's complement (for backward jump)
            switch (ir & 0xe0) {
            case PFX_UDJ:                                 // 0x80 unconditional jump
                pc = PTR(a);                              // set jump target
                break;
            case PFX_CDJ:                                 // 0xa0 conditional jump
                pc = POP() ? pc+1 : PTR(a);               // next or target
                break;
            case PFX_CALL:                                // 0xd0 word call
                RPUSH(IDX(pc+1));                         // keep next instruction on return stack
                pc = PTR(a);                              // jump to subroutine till I_RET
                break;
            case PFX_PRM:                                 // 0xe0 primitive
                _primitive(op);                           // call primitve function with opcode
            }
        }
        NanoForth::yield();
    }
}
//
// execute a primitive opcode
//
void N4VM::_primitive(U8 op)
{
    switch (op) {
    case 0:  POP();                      break; // DRP
    case 1:  PUSH(TOS);                  break; // DUP
    case 2:  {                                  // SWP
        U16 x = TOS1;
        TOS1  = TOS;
        TOS   = x;
    } break;
    case 3:  PUSH(TOS1);                 break; // OVR
    case 4:	 TOS += POP();               break; // +
    case 5:	 TOS -= POP();               break; // -
    case 6:	 TOS *= POP();               break; // *
    case 7:	 TOS /= POP();               break; // /
    case 8:	 TOS %= POP();               break; // MOD
    case 9:  TOS = -TOS;                 break; // NEG
    case 10: TOS &= POP();               break;	// AND
    case 11: TOS |= POP();               break;	// OR
    case 12: TOS ^= POP();               break; // XOR
    case 13: TOS = TOS ? -1 : 0;         break;	// NOT
    case 14: TOS = POP()==TOS;           break; // =
    case 15: TOS = POP()> TOS;           break; // <
    case 16: TOS = POP()< TOS;           break; // >
    case 17: TOS = POP()>=TOS;           break; // <=
    case 18: TOS = POP()<=TOS;           break; // >=
    case 19: TOS = POP()!=TOS;           break; // <>
    case 20: { U8 *p = PTR(POP()); PUSH(GET16(p));  } break; // @
    case 21: { U8 *p = PTR(POP()); SET16(p, POP()); } break; // !
    case 22: { U8 *p = PTR(POP()); PUSH((U16)*p);   } break; // C@
    case 23: { U8 *p = PTR(POP()); *p = (U8)POP();  } break; // C!
    case 24: N4Util::putnum(POP()); putchr(' ');      break; // .
    case 25: {	                                // LOP
        (*(rp-2))++;                      // counter+1
        PUSH(*(rp-2) >= *(rp-1));         // range checkU8
    } break;
    case 26: PUSH(*(rp-2));              break; // I
    case 27: RPOP(); RPOP();             break; // RD2
    case 28: RPUSH(POP()); RPUSH(POP()); break; // P2R2
    // the following 3 opcodes change pc, done at one level up
    case 29: /* used by I_RET */         break;
    case 30: /* used by I_LIT */         break;
    case 31: /* used by I_EXT */         break;
    }
}
//
// extended opcode dispatcher
//
void N4VM::_extended(U8 op)
{
    switch (op) {
    case 0:  /* handled at one level up */    break; // ."
    case 1:  RPUSH(POP());                    break; // >R
    case 2:  PUSH(RPOP());                    break; // R>
    case 3:  n4asm->words();                  break; // WRD
    case 4:  PUSH(IDX(n4asm->here));          break; // HRE
    case 5:  PUSH(POP()*sizeof(U16));         break; // CEL
    case 6:  n4asm->here += POP();            break; // ALO
    case 7:  n4asm->save();                   break; // SAV
    case 8:  n4asm->load();                   break; // LD
    case 9:  set_trace(POP());                break; // TRC
    case 10: /* todo */                       break; // D+
    case 11: /* todo */                       break; // D-
    case 12: /* todo */                       break; // CLK
    case 13: NanoForth::wait((U32)POP());     break; // DLY
    case 14: pinMode(POP(), POP());           break; // PIN
    case 15: PUSH(digitalRead(POP()));        break; // IN
    case 16: digitalWrite(POP(), POP());      break; // OUT
    case 17: PUSH(analogRead(POP()));         break; // AIN
    case 18: analogWrite(POP(), POP());       break; // PWM
    }
}



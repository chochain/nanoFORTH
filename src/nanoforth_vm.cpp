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
#define SS(i)          (*(sp+(i)))                  /**< pointer to the nth on stack         */
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
int N4VM::init(U8 *mem, U16 mem_sz, U16 stk_sz)
{
    n4asm = new N4Asm();     // create and initialze assembler
    if (!n4asm) return 1;
    
    n4asm->init(mem);
    
    dic = &mem[0];
    msz = mem_sz;
    ssz = stk_sz;
    
    _init();

    return 0;
}

///
/// * show system info
///
void N4VM::info()
{
    U16 free = IDX(&free) - IDX(sp);
#if ARDUINO
    putstr(" dic=");       N4Util::d_ptr(dic);
    putstr("[...rp=");     N4Util::d_ptr((U8*)rp);
    putstr("...");         N4Util::d_ptr((U8*)sp);
    putstr("=sp]...");     N4Util::d_ptr((U8*)&free);
#endif // ARDUINO
    putstr(" Free=");      N4Util::putnum(free);
    putstr(" bytes\n");
}
///
/// * virtual machine execute single step
///
void N4VM::step() {
    _ok();                                       // stack check and prompt OK
    
    U8  *tkn = N4Util::token(trc);               // get a token from console
    U16 tmp;
    switch (n4asm->parse_token(tkn, &tmp, 1)) {  ///> parse action from token (keep opcode in tmp)
    case TKN_IMM:                                ///>> immediate words
        switch (tmp) {
        case 0:	n4asm->compile(rp);     break;   ///>>> : (COLON), switch into compile mode (for new word)
        case 1:	n4asm->variable();      break;   ///>>> VAR, create new variable
        case 2: n4asm->constant(POP()); break;   ///>>> CST, create new constant
        case 3:	n4asm->forget();        break;   ///>>> FGT, rollback word created
        case 4: N4Util::dump(                    ///>>> DMP, memory dump
            dic,
            PTR(POP()&0xffe0),
            (POP()+0x1f)&0xffe0);       break;
#if ARDUINO
        case 5: _init();                break;   ///>>> BYE, restart the virtual machine
#else
        case 5: exit(0);                break;   ///>>> BYE, bail!
#endif //ARDUINO
        }                                 break;
    case TKN_DIC: _execute(tmp + 2 + 3);  break; ///>> execute word from dictionary (user defined)
    case TKN_PRM: _primitive((U8)tmp);    break; ///>> execute primitive built-in word
    case TKN_NUM: PUSH(tmp);              break; ///>> push a number (literal) to stack top
    default:                                     ///>> or, error (unknow action)
        putstr("?\n");
    }
}
///
/// * enable/disable execution tracing
///
void N4VM::set_trace(U16 f)
{
    n4asm->set_trace(f);
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

#if ARDUINO
    trc = 0;
#else
    trc = 1;
#endif //ARDUINO
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
	RPUSH(0xffff);                                        // enter function call
    for (U8 *pc=PTR(adr); pc!=PTR(0xffff); ) {            ///> walk through instruction sequences
        U16 a  = IDX(pc);                                 // current program counter
        U8  ir = *pc++;                                   // fetch instruction

        if (trc) n4asm->trace(a, ir);                     // executioU8n tracing when enabled

        U8  op = ir & CTL_BITS;                           ///> determine control bits
        switch (op) {
        case 0xc0:                                        ///> handle branching instruction
            a = GET16(pc-1) & ADR_MASK;                   // target address
            switch (ir & JMP_MASK) {					  // get branch opcode
            case PFX_CALL:                                // 0xc0 subroutine call
                RPUSH(IDX(pc+1));                         // keep next instruction on return stack
                pc = PTR(a);                              // jump to subroutine till I_RET
                break;
            case PFX_RET:                                 // 0xd0 return from subroutine
                a  = RPOP();                              // pop return address
                pc = PTR(a);                              // caller's next instruction (or break loop if 0xffff)
                break;
            case PFX_CDJ:                                 // 0xe0 conditional jump
                pc = POP() ? pc+1 : PTR(a);               // next or target
                break;
            case PFX_UDJ:                                 // 0xf0 unconditional jump
                pc = PTR(a);                              // set jump target
                break;
            }
            break;
        case 0x80:                                        ///> handle primitive word
        	op = ir & PRM_MASK;                           // capture opcode
            switch(op) {
            case I_LIT: PUSH(GET16(pc)); pc+=2; break;    // 3-byte literal
            case I_DQ:  D_STR(pc); pc+=*pc+1;   break;    // handle ." (len,byte,byte,...)
            default: _primitive(op);                      // handle other opcodes
            }
            break;
        default: PUSH(ir);                                ///> handle number (1-byte literal)
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
    case 0:  POP();                       break; // DRP
    case 1:  PUSH(TOS);                   break; // DUP
    case 2:  {                                   // SWP
        U16 x = SS(1);
        SS(1) = TOS;
        TOS   = x;
    } break;
    case 3:  PUSH(SS(1));                 break; // OVR
	case 4:  {                                   // ROT
		U16 x = SS(2);
		SS(2) = SS(1);
		SS(1) = TOS;
		TOS   = x;
	} break;
    case 5:	 TOS += POP();                break; // +
    case 6:	 TOS -= POP();                break; // -
    case 7:	 TOS *= POP();                break; // *
    case 8:	 TOS /= POP();                break; // /
    case 9:  TOS %= POP();                break; // MOD
    case 10: TOS = -TOS;                  break; // NEG
    case 11: TOS &= POP();                break; // AND
    case 12: TOS |= POP();                break; // OR
    case 13: TOS ^= POP();                break; // XOR
    case 14: TOS = TOS ? 0 : 1;           break; // NOT
    case 15: TOS = POP()==TOS;            break; // =
    case 16: TOS = POP()> TOS;            break; // <
    case 17: TOS = POP()< TOS;            break; // >
    case 18: TOS = POP()>=TOS;            break; // <=
    case 19: TOS = POP()<=TOS;            break; // >=
    case 20: TOS = POP()!=TOS;            break; // <>
    case 21: { U8 *p = PTR(POP()); PUSH(GET16(p));  } break; // @
    case 22: { U8 *p = PTR(POP()); SET16(p, POP()); } break; // !
    case 23: { U8 *p = PTR(POP()); PUSH((U16)*p);   } break; // C@
    case 24: { U8 *p = PTR(POP()); *p = (U8)POP();  } break; // C!
    case 25: PUSH((U16)NanoForth::key()); break; // KEY
	case 26: D_CHR((U8)POP());            break; // EMT
    case 27: D_CHR('\n');                 break; // CR
    case 28: N4Util::putnum(POP()); putchr(' ');      break; // .
    case 29: /* handle one level up */    break; // ."
    case 30: RPUSH(POP());                break; // >R
    case 31: PUSH(RPOP());                break; // R>
    case 32: n4asm->words();              break; // WRD
    case 33: PUSH(IDX(n4asm->here));      break; // HRE
    case 34: PUSH(POP()*sizeof(U16));     break; // CEL
    case 35: n4asm->here += POP();        break; // ALO
    case 36: n4asm->save();               break; // SAV
    case 37: n4asm->load();               break; // LD
    case 38: set_trace(POP());            break; // TRC
    case 39: {                                   // CLK
        U32 u = millis();    // Arduino clock
        PUSH((U16)(u>>16));
        PUSH((U16)(u&0xffff));
    }                                     break;
    case 40: {                                   // D+
        S32 v = *(S32*)SS(2) + *(S32*)TOS;
        POP(); POP();
        SS(1) = (S16)(v>>16);
        TOS   = (S16)v&0xffff;
    }                                     break;
    case 41: {                                   // D-
        S32 v = *(S32*)SS(2) - *(S32*)TOS;
        POP(); POP();
        SS(1) = (S16)(v>>16);
        TOS   = (S16)(v&0xffff);
    }                                     break;
    case 42: {                                   // DNG
        S32 v = -(*(S32*)sp);
        SS(1) = (S16)(v>>16);
        TOS   = (S16)(v&0xffff);
    }                                     break;
    case 43: NanoForth::wait((U32)POP()); break; // DLY
#if ARDUINO
    case 44: pinMode(TOS, SS(1));      POP(); POP(); break; // PIN
    case 45: PUSH(digitalRead(POP()));               break; // IN
    case 46: digitalWrite(TOS, SS(1)); POP(); POP(); break; // OUT
    case 47: PUSH(analogRead(POP()));                break; // AIN
    case 48: analogWrite(TOS, SS(1));  POP(), POP(); break; // PWM
#endif //ARDUINO
        /* case 48-58 available for future expansion */
    case 59: RPUSH(POP()); RPUSH(POP());  break; // FOR
    case 60: {	                                 // NXT
        (*(rp-2))++;               // counter+1
        PUSH(*(rp-2) >= *(rp-1));  // range checkU8
    } break;
    case 61: RPOP(); RPOP();              break; // BRK
    case 62: PUSH(*(rp-2));               break; // I
    case 63: /* handled one level up */   break; // LIT
    }
}



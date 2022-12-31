/**
 * @file nanoforth_vm.cpp
 * @brief nanoForth Virtual Machine class implementation
 *
 * #### Forth VM stack opcode macros (notes: rp grows upward and may collide with sp)
 *
 * @code
 *                                    SP0 (sp max to protect overwritten of vm object)
 *        mem[...dic_sz...[...stk_sz...]
 *           |            |            |
 *           dic-->       +->rp    sp<-+
 *                                 TOS TOS1 (top of stack)
 * @endcode
 */
#include "nanoforth_asm.h"
#include "nanoforth_vm.h"
///
///@name Data Stack and Return Stack Ops
///@{
#define SP0            ((S16*)tib)                  /**< top of parameter stack              */
#define TOS            (*sp)                        /**< pointer to top of current stack     */
#define SS(i)          (*(sp+(i)))                  /**< pointer to the nth on stack         */
#define PUSH(v)        (*(--sp)=(S16)(v))           /**< push v onto parameter stack         */
#define POP()          (sp<SP0 ? *sp++ : 0)         /**< pop value off parameter stack       */
#define RPUSH(a)       (*(rp++)=(U16)(a))           /**< push address onto return stack      */
#define RPOP()         (*(--rp))                    /**< pop address from return stack       */
///@}
///@name Dictionary Index <=> Pointer Converters
///@{
#define PTR(n)         ((U8*)dic + (n))             /**< convert dictionary index to a memory pointer */
#define IDX(p)         ((U16)((U8*)(p) - dic))      /**< convert memory pointer to a dictionary index */
///@}
///
///> constructor and initializer
///
N4VM::N4VM(Stream &io, U8 ucase, U8 *dic, U16 dic_sz, U16 stk_sz) :
    n4asm(new N4Asm()), dsz(dic_sz)
{
    set_mem(dic, dic_sz, stk_sz);
    set_io(&io);             /// * set IO stream pointer (static member, shared with N4ASM)
    set_ucase(ucase);        /// * set case sensitiveness

    if (n4asm) _init();      /// * bail if creation failed
}
///
///> show system memory allocation info
///
void N4VM::meminfo()
{
    S16 free = IDX(&free) - IDX(sp);               // in bytes
#if ARDUINO && MEM_DEBUG
    show("mem[");         d_ptr(dic);
    show("=dic|0x");      d_adr((U16)((U8*)rp - dic));
    show("|rp->0x");      d_adr((U16)((U8*)sp - (U8*)rp));
    show("<-sp|tib=");    d_num(free);
    show("..|max=");      d_ptr((U8*)&free);
#endif // MEM_DEBUG
    show("]\n");
}
///
///> virtual machine execute single step (outer interpreter)
/// @return
///  1: more token(s) in input buffer<br/>
///  0: buffer empty (yield control back to hardware)
///
U8 N4VM::step()
{
    if (is_tib_empty()) _ok();                   ///> console ok prompt

    U8  *tkn = get_token();                      ///> get a token from console
    U16 tmp;                                     /// * word address or numeric value
    switch (n4asm->parse_token(tkn, &tmp, 1)) {  ///> parse action from token (keep opcode in tmp)
    case TKN_IMM:                                ///>> immediate words,
        switch (tmp) {
        case 0: n4asm->compile(rp);     break;   /// * : (COLON), switch into compile mode (for new word)
        case 1: n4asm->variable();      break;   /// * VAR, create new variable
        case 2: n4asm->constant(POP()); break;   /// * CST, create new constant
        case 3: n4asm->forget();        break;   /// * FGT, rollback word created
        case 4: _dump(POP(), POP());    break;   /// * DMP, memory dump
        case 5: _init();                break;   /// * RST, restart the virtual machine (for debugging)
#if ARDUINO
        case 6: _init();                break;   /// * BYE, restart
#else
        case 6: exit(0);                break;   /// * BYE, bail to OS
#endif // ARDUINO
        }                               break;
    case TKN_WRD: _nest(tmp + 2 + 3);   break;   ///>> execute colon word (user defined)
    case TKN_PRM: _invoke((U8)tmp);     break;   ///>> execute primitive built-in word,
    case TKN_NUM: PUSH(tmp);            break;   ///>> push a number (literal) to stack top,
    default:                                     ///>> or, error (unknown action)
        show("?\n");
    }
    return !is_tib_empty();                      // stack check and prompt OK
}
///
///> reset virtual machine
///
void N4VM::_init() {
    show("nanoForth v1.6 ");             /// * show init prompt

    rp = (U16*)(dic + dsz);              /// * reset return stack pointer
    sp = SP0;                            /// * reset data stack pointer

    U16 adr = n4asm->reset();            /// * reload EEPROM and reset assembler
    if (adr != LFA_X) {                  /// * check autorun addr has been setup? (see SEX)
        show("reset\n");
        _nest(adr + 2 + 3);              /// * execute last saved colon word in EEPROM
    }
}
///
///> console prompt with stack dump
///
void N4VM::_ok()
{
    S16 *s0 = SP0;                       /// * fetch top of heap
    if (sp > s0) {                       /// * check stack overflow
        show("OVF!\n");
        sp = s0;                         // reset to top of stack block
    }
    for (S16 *p=s0-1; p >= sp; p--) {    /// * dump stack content
        d_num(*p); d_chr('_');
    }
    show("ok");                         /// * user input prompt
}
///
///> opcode execution unit
///
void N4VM::_nest(U16 adr)
{
    RPUSH(LFA_X);                                         // enter function call
    for (U8 *pc=PTR(adr); pc!=PTR(LFA_X); ) {             ///> walk through instruction sequences
        U16 a  = IDX(pc);                                 // current program counter
        U8  ir = *pc++;                                   // fetch instruction

        n4asm->trace(a, ir);                              // execution tracing when enabled

        U8  op = ir & CTL_BITS;                           ///> determine control bits
        if (op==JMP_OPS) {                                ///> handle branching instruction
            a = GET16(pc-1) & ADR_MASK;                   // target address
            switch (ir & JMP_MASK) {                      // get branch opcode
            case OP_CALL:                                 // 0xc0 subroutine call
                RPUSH(IDX(pc+1));                         // keep next instruction on return stack
                pc = PTR(a);                              // jump to subroutine till I_RET
                break;
            case OP_CDJ:                                  // 0xd0 conditional jump
                pc = POP() ? pc+1 : PTR(a);               // next or target
                break;
            case OP_UDJ:                                  // 0xe0 unconditional jump
                pc = PTR(a);                              // set jump target
                break;
            case OP_RET:                                  // 0xf0 return from subroutine
                a  = RPOP();                              // pop return address
                pc = PTR(a);                              // caller's next instruction (or break loop if 0xffff)
                break;
            }
        }
        else if (op==PRM_OPS) {                           ///> handle primitive word
            op = ir & PRM_MASK;                           // capture opcode
            switch(op) {
            case I_NXT:
                if (!--(*(rp-1))) {                       // decrement counter *(rp-1)
                    pc+=2;                                // if (i==0) break loop
                    RPOP();                               // pop off index
                }
                break;
            case I_LIT: PUSH(GET16(pc)); pc+=2; break;    // 3-byte literal
            case I_DQ:  d_str(pc); pc+=*pc+1;   break;    // handle ." (len,byte,byte,...)
            default: _invoke(op);                         // handle other opcodes
            }
        }
        else PUSH(ir);                                    ///> handle number (1-byte literal)

        NanoForth::yield();                               ///> give user task some cycles
    }
}
///
///> invoke a built-in opcode
///
void N4VM::_invoke(U8 op)
{
#define HI16(u)    ((U16)((u)>>16))
#define LO16(u)    ((U16)((u)&0xffff))
#define TO32(u, v) (((S32)(u)<<16) | LO16(v))
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
    case 5:  TOS += POP();                break; // +
    case 6:  TOS -= POP();                break; // -
    case 7:  TOS *= POP();                break; // *
    case 8:  TOS /= POP();                break; // /
    case 9:  TOS %= POP();                break; // MOD
    case 10: TOS = -TOS;                  break; // NEG
    case 11: TOS &= POP();                break; // AND
    case 12: TOS |= POP();                break; // OR
    case 13: TOS ^= POP();                break; // XOR
    case 14: TOS ^= -1;                   break; // NOT
    case 15: TOS <<= POP();               break; // LSH
    case 16: TOS >>= POP();               break; // RSH
    case 17: TOS = POP()==TOS;            break; // =
    case 18: TOS = POP()> TOS;            break; // <
    case 19: TOS = POP()< TOS;            break; // >
    case 20: TOS = POP()!=TOS;            break; // <>
    case 21: { U8 *p = PTR(POP()); PUSH(GET16(p));  } break; // @
    case 22: { U8 *p = PTR(POP()); SET16(p, POP()); } break; // !
    case 23: { U8 *p = PTR(POP()); PUSH((U16)*p);   } break; // C@
    case 24: { U8 *p = PTR(POP()); *p = (U8)POP();  } break; // C!
    case 25: PUSH((U16)key());            break; // KEY
    case 26: d_chr((U8)POP());            break; // EMT
    case 27: d_chr('\n');                 break; // CR
    case 28: d_num(POP()); d_chr(' ');    break; // .
    case 29: /* handled one level up */   break; // ."
    case 30: RPUSH(POP());                break; // >R
    case 31: PUSH(RPOP());                break; // R>
    case 32: n4asm->words();              break; // WRD
    case 33: PUSH(IDX(n4asm->here));      break; // HRE
    case 34: PUSH(POP()*sizeof(U16));     break; // CEL
    case 35: n4asm->here += POP();        break; // ALO
    case 36: n4asm->save();               break; // SAV
    case 37: n4asm->load();               break; // LD
    case 38: n4asm->save(true);           break; // SEX - save/execute (autorun)
    case 39: set_trace(POP());            break; // TRC
    case 40: {                                   // CLK
        U32 u = millis();       // millisecond (32-bit value)
        PUSH(LO16(u));
        PUSH(HI16(u));
    }                                     break;
    case 41: {                                   // D+
        S32 v = TO32(SS(2), SS(3)) + TO32(TOS, SS(1));
        POP(); POP();
        SS(1) = (S16)LO16(v);
        TOS   = (S16)HI16(v);
    }                                     break;
    case 42: {                                   // D-
        S32 v = TO32(SS(2), SS(3)) - TO32(TOS, SS(1));
        POP(); POP();
        SS(1) = (S16)LO16(v);
        TOS   = (S16)HI16(v);
    }                                     break;
    case 43: {                                   // DNG
        S32 v = -TO32(TOS, SS(1));
        SS(1) = (S16)LO16(v);
        TOS   = (S16)HI16(v);
    }                                     break;
#if ARDUINO
    case 44: NanoForth::wait((U32)POP());             break; // DLY
    case 45: PUSH(digitalRead(POP()));                break; // IN
    case 46: PUSH(analogRead(POP()));                 break; // AIN
    case 47: { U16 p=POP(); digitalWrite(p, POP()); } break; // OUT
    case 48: { U16 p=POP(); analogWrite(p, POP());  } break; // PWM
    case 49: { U16 p=POP(); pinMode(p, POP());      } break; // PIN
#endif //ARDUINO
    case 50: TOS = abs(TOS);                          break; // ABS
    case 51: /* available ... */          break;
    case 59: /* ... available */          break;
    case 60: PUSH(*(rp-1));               break; // I
    case 61: RPUSH(POP());                break; // FOR
    case 62: /* handled one level up */   break; // NXT
    case 63: /* handled one level up */   break; // LIT
    }
}
///
///> show a section of memory in Forth dump format
///
void N4VM::_dump(U16 p0, U16 sz0)
{
#if MEM_DEBUG
    U8  *p = PTR((p0&0xffe0));
    U16 sz = (sz0+0x1f)&0xffe0;
    for (U16 i=0; i<sz; i+=0x20) {
        d_mem(dic, p, 0x20, ' ');
        d_chr(' ');
        for (U8 j=0; j<0x20; j++, p++) {         // print and advance to next byte
            char c = *p & 0x7f;
            d_chr((c==0x7f||c<0x20) ? '_' : c);
        }
        d_chr('\n');
    }
#endif // MEM_DEBUG
}

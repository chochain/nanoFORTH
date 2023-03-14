/**
 * @file nanoforth_vm.cpp
 * @brief nanoForth Virtual Machine class implementation
 *
 * #### Forth VM stack opcode macros (notes: rp grows upward and may collide with sp)
 *
 * @code memory space
 *                                    SP0 (sp max to protect overwritten of vm object)
 *        mem[...dic_sz...|...stk_sz...|......heap......]max
 *           |            |            |                |
 *           dic-->       +-->rp  sp<--+-->tib   auto<--+
 *                                TOS TOS1 (top of stack)
 * @endcode
 */
#include "nanoforth_core.h"
#include "nanoforth_asm.h"
#include "nanoforth_intr.h"
#include "nanoforth_vm.h"

using namespace N4Core;                             /// * VM built with core units
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

namespace N4VM {
///
///> invoke a built-in opcode
///
void _invoke(U8 op)
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
    case 32: N4Asm::words();              break; // WRD
    case 33: PUSH(IDX(N4Asm::here));      break; // HRE
    case 34: PUSH(POP()*sizeof(U16));     break; // CEL
    case 35: N4Asm::here += POP();        break; // ALO
    case 36: N4Asm::save();               break; // SAV
    case 37: N4Asm::load();               break; // LD
    case 38: N4Asm::save(true);           break; // SEX - save/execute (autorun)
    case 39: trc = POP();                 break; // TRC
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
    case 44: TOS = abs(TOS);                          break; // ABS
    case 45: set_hex(1);                              break; // HEX
    case 46: set_hex(0);                              break; // DEC
    case 47: { S16 n=POP(); TOS = n>TOS ? n : TOS; }  break; // MAX
    case 48: { S16 n=POP(); TOS = n<TOS ? n : TOS; }  break; // MIN
    case 49: NanoForth::wait((U32)POP());             break; // DLY
    case 50: PUSH(digitalRead(POP()));                break; // IN
    case 51: PUSH(analogRead(POP()));                 break; // AIN
    case 52: { U16 p=POP(); digitalWrite(p, POP()); } break; // OUT
    case 53: { U16 p=POP(); analogWrite(p, POP());  } break; // PWM
    case 54: { U16 p=POP(); pinMode(p, POP());      } break; // PIN
    case 55: N4Intr::enable_timer(POP());             break; // TME - enable/disable timer2 interrupt
    case 56: N4Intr::enable_pci(POP());               break; // PCE - enable/disable pin change interrupts
    case 57: NanoForth::api(POP());                   break; // API
    case 58: case 59:                                 break; // available
    case I_I:   PUSH(*(rp-1));                        break; // I
    case I_FOR: RPUSH(POP());                         break; // FOR
    case I_NXT: /* handled at upper level */          break; // NXT
    case I_LIT: /* handled at upper level */          break; // LIT
    }
}
///
///> opcode execution unit i.e. inner interpreter
///
void _nest(U16 xt)
{
    RPUSH(LFA_X);                                         // enter function call
    for (U8 *pc=PTR(xt), *ex=PTR(LFA_X); pc!=ex; ) {      ///> walk through instruction sequences
        U16 a  = IDX(pc);                                 // current program counter
        U8  ir = *pc++;                                   // fetch instruction

#if    TRC_LEVEL > 0
        if (trc) N4Asm::trace(a, ir);                     // execution tracing when enabled
#endif // TRC_LEVEL

        U8  op = ir & CTL_BITS;                           ///> determine control bits
        if (op==JMP_OPS) {                                ///> handle branching instruction
            a = GET16(pc-1) & ADR_MASK;                   // target address
            switch (ir & JMP_MASK) {                      // get branch opcode
            case OP_CALL:                                 // 0xc0 subroutine call
                NanoForth::yield();         	 		  ///> give user task some cycles (800us)
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
                NanoForth::yield();         	 		  ///> give user task some cycles (800us)
                break;
            case I_LIT: PUSH(GET16(pc)); pc+=2; break;    // 3-byte literal
            case I_DQ:  d_str(pc); pc+=*pc+1;   break;    // handle ." (len,byte,byte,...)
            default:    _invoke(op);                      // handle other opcodes
            }
        }
        else PUSH(ir);                                    ///> handle number (1-byte literal)
    }
}
///
///> reset virtual machine
///
void _init() {
    show(APP_NAME); show(APP_VERSION);   /// * show init prompt

    rp = (U16*)(dic + N4_DIC_SZ);        /// * reset return stack pointer
    sp = SP0;                            /// * reset data stack pointer
    N4Intr::reset();                     /// * init interrupt handler

    U16 xt = N4Asm::reset();             /// * reload EEPROM and reset assembler
    if (xt != LFA_X) {                   /// * check autorun addr has been setup? (see SEX)
        show("reset\n");
        _nest(xt + 2 + 3);               /// * execute last saved colon word in EEPROM
    }
}
///
///> show a section of memory in Forth dump format
///
#define DUMP_PER_LINE 0x10
void _dump(U16 p0, U16 sz0)
{
    U8  *p = PTR((p0&0xffe0));
    U16 sz = (sz0+0x1f)&0xffe0;
    for (U16 i=0; i<sz; i+=DUMP_PER_LINE) {
        d_chr('\n');
        d_mem(dic, p, DUMP_PER_LINE, ' ');
        d_chr(' ');
        for (U8 j=0; j<DUMP_PER_LINE; j++, p++) {         // print and advance to next byte
            char c = *p & 0x7f;
            d_chr((c==0x7f||c<0x20) ? '_' : c);
        }
    }
}
///
///> constructor and initializer
///
void setup(Stream &io, U8 ucase)
{
    init_mem();
    memstat();               ///< display VM system info

    set_io(&io);             /// * set IO stream pointer (static member, shared with N4ASM)
    set_ucase(ucase);        /// * set case sensitiveness
    set_hex(0);              /// * set radix = 10

    _init();      			 /// * init VM
}
///
///> virtual machine interrupt service routine
///
void serv_isr() {
	U16 xt = N4Intr::isr();
	if (xt) _nest(xt);
}
///
///> virtual machine execute single step (outer interpreter)
/// @return
///  1: more token(s) in input buffer<br/>
///  0: buffer empty (yield control back to hardware)
///
void outer()
{
    ok();                                        ///> console ok prompt if tib is empty
    U8  *tkn = get_token();                      ///> get a token from console
    U16 tmp;                                     /// * word address or numeric value
    switch (N4Asm::parse(tkn, &tmp, 1)) {        ///> parse action from token (keep opcode in tmp)
    case TKN_IMM:                                ///>> immediate words,
        switch (tmp) {
        case 0: N4Asm::compile(rp);     break;   /// * : (COLON), switch into compile mode (for new word)
        case 1: N4Asm::variable();      break;   /// * VAR, create new variable
        case 2: N4Asm::constant(POP()); break;   /// * CST, create new constant
        case 3: N4Intr::add_pcisr(               /// * PCI, create a pin change interrupt handler
                POP(), N4Asm::query()); break;
        case 4: N4Intr::add_tmisr(               /// * TMR, create a timer interrupt handler
                POP(), N4Asm::query()); break;   /// * period in 0.1 sec
        case 5: N4Asm::forget();        break;   /// * FGT, rollback word created
        case 6: _dump(POP(), POP());    break;   /// * DMP, memory dump
        case 7: _init();                break;   /// * RST, restart the virtual machine (for debugging)
#if ARDUINO
        case 8: _init();                break;   /// * BYE, restart
#else
        case 8: exit(0);                break;   /// * BYE, bail to OS
#endif // ARDUINO
        }                               break;
    case TKN_WRD: _nest(tmp + 2 + 3);   break;   ///>> execute colon word (user defined)
    case TKN_PRM: _invoke((U8)tmp);     break;   ///>> execute primitive built-in word,
    case TKN_NUM: PUSH(tmp);            break;   ///>> push a number (literal) to stack top,
    case TKN_EXT:                                ///>> extended words, not implemented yet
    default:                                     ///>> or, error (unknown action)
        show("?\n");
    }
}

} // namespace N4VM

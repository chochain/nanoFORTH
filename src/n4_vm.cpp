/**
 * @file
 * @brief nanoForth Virtual Machine class implementation
 *
 * #### Forth VM stack opcode macros (notes: rp grows upward and may collide with sp)
 *
 * @code memory space
 *                                    SP0 (sp max to protect overwritten of vm object)
 *        mem[...dic_sz...|...stk_sz...|......heap......]max
 *           |            |            |                |
 *           dic-->       +-->rp  sp<--+-->tib   auto<--+
 *                                TOS NOS
 * @endcode
 */
#include "n4_core.h"
#include "n4_asm.h"
#include "n4_intr.h"
#include "n4_vm.h"

using namespace N4Core;                             /// * VM built with core units
///
///@name Data Stack and Return Stack Ops
///@{
#define SP0            ((S16*)&dic[N4_DIC_SZ + N4_STK_SZ])
#define TOS            (*sp)                        /**< pointer to top of current stack     */
#define SS(i)          (*(sp+(i)))                  /**< pointer to the nth on stack         */
#define PUSH(v)        (*(--sp)=(S16)(v))           /**< push v onto parameter stack         */
#define POP()          (*sp++)                      /**< pop value off parameter stack       */
#define RPUSH(a)       (*(rp++)=(U16)(a))           /**< push address onto return stack      */
#define RPOP()         (*(--rp))                    /**< pop address from return stack       */
///@}
///@name Dictionary Index <=> Pointer Converters
///@{
#define DIC(n)         ((U8*)dic + (n))             /**< convert dictionary index to a memory pointer */
#define IDX(p)         ((U16)((U8*)(p) - dic))      /**< convert memory pointer to a dictionary index */
///@}
namespace N4VM {
///
///> reset virtual machine
///
void _nest(U16 xt);                      /// * forward declaration
void _init() {
    show(APP_NAME); show(APP_VERSION);   /// * show init prompt

    rp = (U16*)DIC(N4_DIC_SZ);           /// * reset return stack pointer
    sp = SP0;                            /// * reset data stack pointer
    N4Intr::reset();                     /// * init interrupt handler

    U16 xt = N4Asm::reset();             /// * reload EEPROM and reset assembler
    if (xt != LFA_END) {                 /// * check autorun addr has been setup? (see SEX)
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
    U8  *p = DIC((p0&0xffe0));
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
///> immediate word handler
///
void _immediate(U16 op)
{
        switch (op) {
        ///> compiler
        case 0: N4Asm::compile(rp);     break;   /// * : (COLON), switch into compile mode (for new word)
        case 1: N4Asm::variable();      break;   /// * VAR, create new variable
        case 2: N4Asm::constant(POP()); break;   /// * VAL, create new constant
        ///> interrupt handlers
        case 3: N4Intr::add_pcisr(               /// * PCI, create a pin change interrupt handler
                POP(), N4Asm::query()); break;
        case 4:                                  /// * TMI, create a timer interrupt handler
            op = POP();                          ///< tmp = ISR slot#
            N4Intr::add_tmisr(
                op, POP(),
                N4Asm::query());        break;   /// * period in multiply of 10ms
        ///> numeric radix
        case 5: set_hex(1);             break;   /// * HEX
        case 6: set_hex(0);             break;   /// * DEC
        ///> dicionary debugging
        case 7: N4Asm::forget();        break;   /// * FGT, rollback word created
        case 8: N4Asm::words();         break;   /// * WRD
        case 9:                                  /// * DMP, memory dump
            op = POP();
            _dump(POP(), op);           break;
        case 10: N4Asm::see();          break;   /// * SEE
        ///> system
        case 11: N4Asm::save();         break;   /// * SAV
        case 12: N4Asm::load();         break;   /// * LD
        case 13: N4Asm::save(true);     break;   /// * SEX - save/execute (autorun)
#if ARDUINO
        case 14: _init();               break;   /// * BYE, restart
#else
        case 14: exit(0);               break;   /// * BYE, bail to OS
#endif // ARDUINO
        }
}
///
///> 32-bit operators
/// @brief: stand-alone functions to reduce register allocation in _invoke
///
#define HI16(u)    ((U16)((u)>>16))
#define LO16(u)    ((U16)((u)&0xffff))
#define TO32(u, v) (((S32)(u)<<16) | LO16(v))
void _clock() {
    U32 u = millis();       // millisecond (32-bit value)
    PUSH(LO16(u));
    PUSH(HI16(u));
}
void _dplus() {
    S32 v = TO32(SS(2), SS(3)) + TO32(TOS, SS(1));
    POP(); POP();
    SS(1) = (S16)LO16(v);
    TOS   = (S16)HI16(v);
}
void _dminus() {
    S32 v = TO32(SS(2), SS(3)) - TO32(TOS, SS(1));
    POP(); POP();
    SS(1) = (S16)LO16(v);
    TOS   = (S16)HI16(v);
}
void _dneg() {
    S32 v = -TO32(TOS, SS(1));
    SS(1) = (S16)LO16(v);
    TOS   = (S16)HI16(v);
}
///
///> invoke a built-in opcode
///
void _invoke(U8 op)
{
    switch (op) {
    case 0: /* handled at upper level */  break; // NOP
    case 1:  POP();                       break; // DRP
    case 2:  PUSH(TOS);                   break; // DUP
    case 3:  {                                   // SWP
        U16 x = SS(1);
        SS(1) = TOS;
        TOS   = x;
    } break;
    case 4:  PUSH(SS(1));                 break; // OVR
    case 5:  {                                   // ROT
        U16 x = SS(2);
        SS(2) = SS(1);
        SS(1) = TOS;
        TOS   = x;
    } break;
    case 6:  TOS += POP();                break; // +
    case 7:  TOS -= POP();                break; // -
    case 8:  TOS *= POP();                break; // *
    case 9:  TOS /= POP();                break; // /
    case 10: TOS %= POP();                break; // MOD
    case 11: TOS = -TOS;                  break; // NEG
    case 12: TOS &= POP();                break; // AND
    case 13: TOS |= POP();                break; // OR
    case 14: TOS ^= POP();                break; // XOR
    case 15: TOS ^= -1;                   break; // NOT
    case 16: TOS <<= POP();               break; // LSH
    case 17: TOS >>= POP();               break; // RSH
    case 18: TOS = POP()==TOS;            break; // =
    case 19: TOS = POP()> TOS;            break; // <
    case 20: TOS = POP()< TOS;            break; // >
    case 21: TOS = POP()!=TOS;            break; // <>
    case 22: { U8 *p = DIC(POP()); PUSH(GET16(p));  } break; // @
    case 23: { U8 *p = DIC(POP()); ENC16(p, POP()); } break; // !
    case 24: { U8 *p = DIC(POP()); PUSH((U16)*p);   } break; // C@
    case 25: { U8 *p = DIC(POP()); *p = (U8)POP();  } break; // C!
    case 26: PUSH((U16)key());            break; // KEY
    case 27: d_chr((U8)POP());            break; // EMT
    case 28: d_chr('\n');                 break; // CR
    case 29: d_num(POP()); d_chr(' ');    break; // .
    case 30: /* handled one level up */   break; // ."
    case 31: RPUSH(POP());                break; // >R
    case 32: PUSH(RPOP());                break; // R>
    case 33: PUSH(IDX(N4Asm::here));      break; // HRE
    case 34: PUSH(random(POP()));         break; // RND
    case 35: N4Asm::here += POP();        break; // ALO
    case 36: trc = POP();                 break; // TRC
    case 37: _clock();                    break; // CLK
    case 38: _dplus();                    break; // D+
    case 39: _dminus();                   break; // D-
    case 40: _dneg();                     break; // DNG
    case 41: TOS = abs(TOS);                          break; // ABS
    case 42: { S16 n=POP(); TOS = n>TOS ? n : TOS; }  break; // MAX
    case 43: { S16 n=POP(); TOS = n<TOS ? n : TOS; }  break; // MIN
    case 44: NanoForth::wait((U32)POP());             break; // DLY
    case 45: PUSH(d_in(POP()));                       break; // IN
    case 46: PUSH(a_in(POP()));                       break; // AIN
    case 47: { U16 p=POP(); d_out(p, POP()); }        break; // OUT
    case 48: { U16 p=POP(); a_out(p, POP()); }        break; // PWM
    case 49: { U16 p=POP(); d_pin(p, POP()); }        break; // PIN
    case 50: N4Intr::enable_timer(POP());             break; // TME - enable/disable timer2 interrupt
    case 51: N4Intr::enable_pci(POP());               break; // PCE - enable/disable pin change interrupts
    case 52: NanoForth::call_api(POP());              break; // API
#if N4_META
    ///> meta programming (for advance users)
    case 53: N4Asm::create();                         break; // CRE, create a word (header only)
    case 54: N4Asm::comma(POP());                     break; // ,    comma, add a 16-bit value onto dictionary
    case 55: N4Asm::ccomma(POP());                    break; // C,   C-comma, add a 8-bit value onto dictionary
    case 56: PUSH(N4Asm::query());                    break; // '    tick, get parameter field of a word
    case 57: _nest(POP());                            break; // EXE  execute a given parameter field
    case 58: /* handled at upper level */             break; // DO>
#endif // N4_META
    /* case 59, 60 available */
    case I_I:   PUSH(*(rp-1));                        break; // 61, I
    case I_FOR: RPUSH(POP());                         break; // 62, FOR
    case I_LIT: /* handled at upper level */          break; // 63, LIT
    }
}
///
///> opcode execution unit i.e. inner interpreter
///
void _nest(U16 xt)
{
    static U8 isr_cnt = 0;                                // interrupt service counter
    RPUSH(LFA_END);                                       // enter function call
    while (xt != LFA_END) {                               ///> walk through instruction sequences
        U8 op = *DIC(xt);                                 // fetch instruction
        if (isr_cnt++) serv_isr();                        // loop-around every 256 ops

#if    TRC_LEVEL > 0
        if (trc) N4Asm::trace(xt, op);                    // execution tracing when enabled
#endif // TRC_LEVEL

        switch (op & CTL_BITS) {                          ///> determine control bits
        case JMP_OPS: {                                   ///> handle branching instruction
            U16 w = GET16(DIC(xt)) & ADR_MASK;            // target address
            switch (op & JMP_MASK) {                      // get branch opcode
            case OP_CALL:                                 // 0xc0 subroutine call
                RPUSH(xt+2);                              // keep next instruction on return stack
                xt = w;                                   // jump to subroutine till I_RET
                break;
            case OP_CDJ: xt = POP() ? xt+2 : w; break;    // 0xd0 conditional jump
            case OP_UDJ: xt = w;                break;    // 0xe0 unconditional jump
            case OP_NXT:                                  // 0xf0 FOR...NXT
                if (!--(*(rp-1))) {                       // decrement counter *(rp-1)
                    xt += 2;                              // break loop
                    RPOP();                               // pop off loop index
                }
                else xt = w;                              // loop back
                break;
            }
        } break;
        case PRM_OPS: {                                   ///> handle primitive word
            xt++;                                         // advance 1 (primitive token)
            op &= PRM_MASK;                               // capture opcode
            switch(op) {
            case I_RET:	xt = RPOP();     break;           // POP return address
            case I_LIT: {                                 // 3-byte literal
                U16 w = GET16(DIC(xt));
                PUSH(w);
                xt += 2;
            }                            break;
            case I_DQ:                                    // handle ." (len,byte,byte,...)
                d_str(DIC(xt));
                xt += *DIC(xt) + 1;      break;
            case I_DO:                                    // metaprogrammer
                N4Asm::does(xt);                          // jump to definding word DO> section
                xt = LFA_END;            break;
            default: _invoke(op);                         // handle other opcodes
            }
        } break;
        default:                                          ///> handle number (1-byte literal)
            xt++;
            PUSH(op);
        }
    }
}
///
///> constructor and initializer
///
void setup(const char *code, Stream &io, U8 ucase)
{
    init_mem();
    memstat();               ///< display VM system info

    set_pre(code);           /// * install embedded Forth code
    set_io(&io);             /// * set IO stream pointer (static member, shared with N4ASM)
    set_ucase(ucase);        /// * set case sensitiveness
    set_hex(0);              /// * set radix = 10

    _init();                   /// * init VM
}
///
///> VM proxy functions
///
void push(int v) { PUSH(v);      }
int  pop()       { return POP(); }
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
    U16 tmp;
    switch (N4Asm::parse(tkn, &tmp, 1)) {        ///> parse action from token (keep opcode in tmp)
    case TKN_IMM: _immediate(tmp);      break;   ///>> immediate words,
    case TKN_WRD: _nest(tmp + 2 + 3);   break;   ///>> execute colon word (user defined)
    case TKN_PRM: _invoke((U8)tmp);     break;   ///>> execute primitive built-in word,
    case TKN_NUM: PUSH(tmp);            break;   ///>> push a number (literal) to stack top,
    case TKN_EXT:                                ///>> extended words, not implemented yet
    default:                                     ///>> or, error (unknown action)
        show("?\n");
    }
}
} // namespace N4VM

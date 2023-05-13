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
#define TOS            (*vm.sp)                     /**< pointer to top of current stack     */
#define SS(i)          (*(vm.sp+(i)))               /**< pointer to the nth on stack         */
#define PUSH(v)        (*(--vm.sp)=(S16)(v))        /**< push v onto parameter stack         */
#define POP()          (*vm.sp++)                   /**< pop value off parameter stack       */
#define RPUSH(a)       (*(vm.rp++)=(U16)(a))        /**< push address onto return stack      */
#define RPOP()         (*(--vm.rp))                 /**< pop address from return stack       */
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

    vm.rp = (U16*)DIC(N4_DIC_SZ);        /// * reset return stack pointer
    vm.sp = SP0;                         /// * reset data stack pointer
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
    U8  *p = DIC(p0 & 0xffe0);
    U16 sz = (sz0 + 0x1f) & 0xffe0;
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
        case 0: N4Asm::compile(vm.rp);  break;   /// * : (COLON), switch into compile mode (for new word)
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
///> Note: computed goto takes extra 128-bytes for ~60ms/100K faster
///
void _invoke(U8 op)
{
#if N4_USE_GOTO
    #define DISPATCH(op) goto *vt[op];
    #define _X(i,g)      L_##i: { g; } return
    #define LL(i) \
        &&L_##i##0,&&L_##i##1,&&L_##i##2,&&L_##i##3,&&L_##i##4, \
        &&L_##i##5,&&L_##i##6,&&L_##i##7,&&L_##i##8,&&L_##i##9
    static void *vt[] = {           // computed goto branching table
        LL(),LL(1),LL(2),LL(3),LL(4),LL(5),&&L_60,&&L_61,&&L_62,&&L_63
    };
#else  // !N4_USE_GOTO
    #define DISPATCH(op) switch(op)
    #define _X(i,g)      case i: { g; } break
#endif // N4_USE_GOTO

    DISPATCH(op) {                  // switch(op) or goto *vt[op]
    _X(0,  {});                     // NOP, handled at upper level
    _X(1,  POP());                  // DRP
    _X(2,  PUSH(TOS));              // DUP
    _X(3,                           // SWP
        U16 x = SS(1);
        SS(1) = TOS;
        TOS   = x);
    _X(4,  PUSH(SS(1)));            // OVR
    _X(5,                           // ROT
        U16 x = SS(2);
        SS(2) = SS(1);
        SS(1) = TOS;
        TOS   = x);
    _X(6,  TOS += POP());           // +
    _X(7,  TOS -= POP());           // -
    _X(8,  TOS *= POP());           // *
    _X(9,  TOS /= POP());           // /
    _X(10, TOS %= POP());           // MOD
    _X(11, TOS = -TOS);             // NEG
    _X(12, TOS &= POP());           // AND
    _X(13, TOS |= POP());           // OR
    _X(14, TOS ^= POP());           // XOR
    _X(15, TOS ^= -1);              // NOT
    _X(16, TOS <<= POP());          // LSH
    _X(17, TOS >>= POP());          // RSH
    _X(18, TOS = POP()==TOS);       // =
    _X(19, TOS = POP()> TOS);       // <
    _X(20, TOS = POP()< TOS);       // >
    _X(21, TOS = POP()!=TOS);       // <>
    _X(22, U8 *p = DIC(POP()); PUSH(GET16(p)) ); // @
    _X(23, U8 *p = DIC(POP()); ENC16(p, POP())); // !
    _X(24, U8 *p = DIC(POP()); PUSH((U16)*p)  ); // C@
    _X(25, U8 *p = DIC(POP()); *p = (U8)POP() ); // C!
    _X(26, PUSH((U16)key()));       // KEY
    _X(27, d_chr((U8)POP()));       // EMT
    _X(28, d_chr('\n'));            // CR
    _X(29, d_num(POP()); d_chr(' ')); // .
    _X(30, {});                     // ."  handled one level up
    _X(31, RPUSH(POP()));           // >R
    _X(32, PUSH(RPOP()));           // R>
    _X(33, PUSH(IDX(N4Asm::here))); // HRE
    _X(34, PUSH(random(POP())));    // RND
    _X(35, N4Asm::here += POP());   // ALO
    _X(36, trc = POP());            // TRC
    _X(37, _clock());               // CLK
    _X(38, _dplus());               // D+
    _X(39, _dminus());              // D-
    _X(40, _dneg());                // DNG
    _X(41, TOS = abs(TOS));         // ABS
    _X(42, S16 n = POP(); TOS = n>TOS ? n : TOS); // MAX
    _X(43, S16 n = POP(); TOS = n<TOS ? n : TOS); // MIN
    _X(44, NanoForth::wait((U32)POP()));          // DLY
    _X(45, PUSH(d_in(POP())));                    // IN
    _X(46, PUSH(a_in(POP())));                    // AIN
    _X(47, U16 p = POP(); d_out(p, POP()));       // OUT
    _X(48, U16 p = POP(); a_out(p, POP()));       // PWM
    _X(49, U16 p = POP(); d_pin(p, POP()));       // PIN
    _X(50, N4Intr::enable_timer(POP()));          // TME - enable/disable timer2 interrupt
    _X(51, N4Intr::enable_pci(POP()));            // PCE - enable/disable pin change interrupts
    _X(52, NanoForth::call_api(POP()));           // API
#if N4_DOES_META
    ///> meta programming (for advance users)
    _X(53, N4Asm::create());        // CRE, create a word (header only)
    _X(54, N4Asm::comma(POP()));    // ,    comma, add a 16-bit value onto dictionary
    _X(55, N4Asm::ccomma(POP()));   // C,   C-comma, add a 8-bit value onto dictionary
    _X(56, PUSH(N4Asm::query()));   // '    tick, get parameter field of a word
    _X(57, _nest(POP()));           // EXE  execute a given parameter field
    _X(58, {});                     // DO> handled at upper level
#endif // N4_DOES_META
    _X(59, {});                     // available
    _X(60, {});                     // available
    _X(61, PUSH(*(vm.rp - 1)));     // 61, I
    _X(62, RPUSH(POP()));           // 62, FOR
    _X(63, {});                     // 63, LIT handled at upper level
    }
}
///
///> opcode execution unit i.e. inner interpreter
///
void _nest(U16 xt)
{
    RPUSH(LFA_END);                                       // enter function call
    while (xt != LFA_END) {                               ///> walk through instruction sequences
        U8 op = *DIC(xt);                                 // fetch instruction

#if    TRC_LEVEL > 0
        if (trc) N4Asm::trace(xt, op);                    // execution tracing when enabled
#endif // TRC_LEVEL

        if ((op & CTL_BITS)==JMP_OPS) {                   ///> determine control bits
            U16 w = (((U16)op<<8) | *DIC(xt+1)) & ADR_MASK;  // target address
            switch (op & JMP_MASK) {                      // get branch opcode
            case OP_CALL:                                 // 0xc0 subroutine call
                serv_isr();                               // loop-around every 256 ops
                RPUSH(xt+2);                              // keep next instruction on return stack
                xt = w;                                   // jump to subroutine till I_RET
                break;
            case OP_CDJ: xt = POP() ? xt+2 : w; break;    // 0xd0 conditional jump
            case OP_UDJ: xt = w;                break;    // 0xe0 unconditional jump
            case OP_NXT:                                  // 0xf0 FOR...NXT
                if (!--(*(vm.rp-1))) {                    // decrement counter *(rp-1)
                    xt += 2;                              // break loop
                    RPOP();                               // pop off loop index
                }
                else xt = w;                              // loop back
                serv_isr();                               // loop-around every 256 ops
                break;
            }
        }
        else if ((op & CTL_BITS)==PRM_OPS) {              ///> handle primitive word
            xt++;                                         // advance 1 (primitive token)
            op &= PRM_MASK;                               // capture opcode
            switch(op) {
            case I_RET:    xt = RPOP();  break;           // POP return address
            case I_LIT: {                                 // 3-byte literal
                U16 w = GET16(DIC(xt));                   // fetch the 16-bit literal
                PUSH(w);                                  // put the value on TOS
                xt += 2;                                  // skip over the 16-bit literal
            }                            break;
            case I_DQ:                                    // handle ." (len,byte,byte,...)
                d_str(DIC(xt));                           // display the string
                xt += *DIC(xt) + 1;      break;           // skip over the string
            case I_DO:                                    // metaprogrammer
                N4Asm::does(xt);                          // jump to definding word DO> section
                xt = LFA_END;            break;
            default: _invoke(op);                         // handle other opcodes
            }
        }
        else {                                            ///> handle number (1-byte literal)
            xt++;
            PUSH(op);                                     // put the 7-bit literal on TOS
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

    _init();                 /// * init VM
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

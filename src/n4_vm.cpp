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
#define SP0            ((DU*)&dic[N4_DIC_SZ + N4_STK_SZ])
#define TOS            (*vm.sp)                     /**< pointer to top of current stack     */
#define SS(i)          (*(vm.sp+(i)))               /**< pointer to the nth on stack         */
#define PUSH(v)        (*(--vm.sp)=(DU)(v))         /**< push v onto parameter stack         */
#define POP()          (*vm.sp++)                   /**< pop value off parameter stack       */
#define RPUSH(a)       (*(vm.rp++)=(IU)(a))         /**< push address onto return stack      */
#define RPOP()         (*(--vm.rp))                 /**< pop address from return stack       */
#define BOOL(f)        ((f) ? -1 : 0)               /**< TRUE=-1 per common Forth idiom      */
///@}
///@name Dictionary Index <=> Pointer Converters
///@{
#define DIC(n)         ((U8*)dic + (n))             /**< convert dictionary index to a memory pointer */
#define IDX(p)         ((IU)((U8*)(p) - dic))       /**< convert memory pointer to a dictionary index */
///@}
namespace N4VM {
///
///> reset virtual machine
///
void _nest(IU xt);                       /// * forward declaration
void _init() {
    mstat();

    vm.rp = (IU*)DIC(N4_DIC_SZ);         /// * reset return stack pointer
    vm.sp = SP0;                         /// * reset data stack pointer
    N4Intr::reset();                     /// * init interrupt handler

    IU xt = N4Asm::reset();              /// * reload EEPROM and reset assembler
    if (xt != LFA_END) {                 /// * check autorun addr has been setup? (see SEX)
        show("reset\n");
        _nest(XT(xt));                   /// * execute last saved colon word in EEPROM
    }
}
///
///> show a section of memory in Forth dump format
///
#define DUMP_PER_LINE 0x10
void _dump(IU p0, U16 sz0)
{
#if TRC_LEVEL > 0
    U8  *p = DIC(p0 & 0xffe0);
    U16 sz = (sz0 + 0x1f) & 0xffe0;
    d_chr('\n');
    for (U16 i=0; i<sz; i+=DUMP_PER_LINE) {
        d_mem(dic, p, DUMP_PER_LINE, ' ');
        d_chr(' ');
        for (int j=0; j<DUMP_PER_LINE; j++, p++) {    // print and advance to next byte
            char c = *p & 0x7f;
            d_chr((c==0x7f||c<0x20) ? '_' : c);
        }
        d_chr('\n');
    }
#endif // TRC_LEVEL > 0    
}
///
///> immediate word handler
///
void _immediate(U16 op)
{
        switch (op) {
        ///> compiler
        case 0: N4Asm::compile(vm.rp);  break;   /// * : (COLON), switch into compile mode (for new word)
        case 1: N4Asm::constant(POP()); break;   /// * VAL, create new constant
        case 2: N4Asm::variable();      break;   /// * VAR, create new variable
        ///> interrupt handlers
        case 3: N4Intr::add_pcisr(               /// * PCI, create a pin change interrupt handler
                POP(), N4Asm::query()); break;
        case 4:                                  /// * TMI, create a timer interrupt handler
            op = POP();                          ///< tmp = ISR slot#
            N4Intr::add_tmisr(
                op, POP(),
                N4Asm::query());        break;   /// * period in multiply of 10ms
        ///> system
        case 5: N4Asm::save(1);         break;   /// * SEX - save/execute (autorun)
        case 6: N4Asm::save();          break;   /// * SAV
        case 7: N4Asm::load();          break;   /// * LD
        ///> dicionary debugging
        case 8: N4Asm::forget();        break;   /// * FGT, rollback word created
        case 9:                                  /// * DMP, memory dump
            op = POP();
            _dump(POP(), op);           break;
        case 10: N4Asm::see();          break;   /// * SEE
        case 11: N4Asm::words();        break;   /// * WRD
        ///> numeric radix
        case 12: set_hex(0);            break;   /// * DEC
        case 13: set_hex(1);            break;   /// * HEX
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
#define S2D(u, v)  (((DU2)(u)<<16) | D_LO(v))
#define D_HI(u)    ((U16)((u)>>16))
#define D_LO(u)    ((U16)((u)&0xffff))
void _clock() {
    DU2 d = millis();       // millisecond (32-bit value)
    PUSH(D_LO(d));
    PUSH(D_HI(d));
}
void _dplus() {
    DU2 d = S2D(SS(2), SS(3)) + S2D(TOS, SS(1));
    POP(); POP();
    SS(1) = (DU)D_LO(d);
    TOS   = (DU)D_HI(d);
}
void _dminus() {
    DU2 d = S2D(SS(2), SS(3)) - S2D(TOS, SS(1));
    POP(); POP();
    SS(1) = (DU)D_LO(d);
    TOS   = (DU)D_HI(d);
}
void _dneg() {
    DU2 d = -S2D(TOS, SS(1));
    SS(1) = (DU)D_LO(d);
    TOS   = (DU)D_HI(d);
}
///
///> invoke a built-in opcode
///> Note: computed goto takes extra 128-bytes for ~5% faster
///
IU _invoke(U8 op, IU xt=0)
{
#if N4_USE_GOTO                     // defined in n4_asm.h
    #define DISPATCH(op) goto *vt[op];
    #define CODE(i,g) L_##i: { g; } return xt
    #define LL(i) \
        &&L_##i##0,&&L_##i##1,&&L_##i##2,&&L_##i##3,&&L_##i##4, \
        &&L_##i##5,&&L_##i##6,&&L_##i##7,&&L_##i##8,&&L_##i##9
    static void *vt[] = {           // computed goto branching table
        LL(),LL(1),LL(2),LL(3),LL(4),LL(5),&&L_60,&&L_61,&&L_62,&&L_63
    };
#else  // !N4_USE_GOTO
    #define DISPATCH(op) switch(op)
    #define CODE(i,g)    case i: { g; } break
#endif // N4_USE_GOTO

    DISPATCH(op) {                 // switch(op) or goto *vt[op]
    ///> stack ops
    CODE(0,  xt = RPOP());                          // ___
    CODE(1,  trc = POP());                          // TRC
    CODE(2,                                         // ROT
         DU x = SS(2); SS(2) = SS(1);
         SS(1)= TOS; TOS = x);
    CODE(3,  PUSH(SS(1)));                          // OVR
    CODE(4,  DU x = SS(1); SS(1) = TOS; TOS = x);   // SWP
    CODE(5,  PUSH(TOS));                            // DUP
    CODE(6,  POP());                                // DRP
    ///> Bit-wise ops
    CODE(7,  TOS <<= POP());                        // LSH
    CODE(8,  DU n = POP(); TOS = (U16)TOS >> n);    // RSH
    CODE(9,  TOS ^= 0xffff);                        // NOT
    CODE(10, TOS ^= POP());                         // XOR
    CODE(11, TOS |= POP());                         // OR
    CODE(12, TOS &= POP());                         // AND
    ///> ALU ops
    CODE(13, PUSH(random(POP())));                  // RND
    CODE(14, DU n = POP(); TOS = n<TOS ? n : TOS);  // MIN
    CODE(15, DU n = POP(); TOS = n>TOS ? n : TOS);  // MAX
    CODE(16, TOS = TOS > 0 ? TOS : -TOS);           // ABS
    CODE(17, TOS %= POP());                         // MOD
    CODE(18, TOS = -TOS);                           // NEG
    CODE(19, TOS /= POP());                         // /
    CODE(20, TOS *= POP());                         // *
    CODE(21, TOS -= POP());                         // -
    CODE(22, TOS += POP());                         // +
    ///> Logical ops
    CODE(23, TOS = BOOL(POP()==TOS));               // =
    CODE(24, TOS = BOOL(POP()> TOS));               // <
    CODE(25, TOS = BOOL(POP()< TOS));               // >
    CODE(26, TOS = BOOL(POP()!=TOS));               // <>
    ///> IO ops
    CODE(27, PUSH((DU)key()));                      // KEY
    CODE(28, d_chr((U8)POP()));                     // EMT
    CODE(29, d_chr('\n'));                          // CR
    CODE(30, d_num(POP()); d_chr(' '));             // .
    CODE(31,                                        // ."
         if (xt) {                                  //   interpreter mode
             d_str(DIC(xt)); xt += *DIC(xt) + 1;    //   display str
         }
         else N4Asm::dot_str());                    //   immediate
    CODE(32,                                        // .S
         PUSH(xt);                                  //   string pointer
         PUSH(*DIC(xt));                            //   strlen
         xt += *DIC(xt) + 1);                       //   skip over str
    CODE(33, POP(); d_str(DIC(POP())));             // TYP
    ///> Compiler ops
    CODE(34, PUSH(IDX(N4Asm::here)));               // HRE
    CODE(35, RPUSH(POP()));                         // >R
    CODE(36, PUSH(RPOP()));                         // R>
    CODE(37, U8 *p = DIC(POP()); STORE(p, POP()));  // !
    CODE(38, U8 *p = DIC(POP()); PUSH(FETCH(p)) );  // @
    CODE(39, U8 *p = DIC(POP()); *p = (U8)POP() );  // C!
    CODE(40, U8 *p = DIC(POP()); PUSH((DU)*p)  );   // C@
    CODE(41, N4Asm::here += POP());                 // ALO
    ///> Double ops
    CODE(42, _dneg());                              // DNG
    CODE(43, _dminus());                            // D-
    CODE(44, _dplus());                             // D+
    CODE(45, _clock());                             // CLK
    ///> Arduino Specific ops
    CODE(46, NanoForth::wait((U32)POP()));          // DLY
    CODE(47, U16 p = POP(); a_out(p, POP()));       // PWM
    CODE(48, U16 p = POP(); d_out(p, POP()));       // OUT
    CODE(49, PUSH(a_in(POP())));                    // AIN
    CODE(50, PUSH(d_in(POP())));                    // IN
    CODE(51, U16 p = POP(); d_pin(p, POP()));       // PIN
    CODE(52, N4Intr::enable_pci(POP()));            // PCE - enable/disable pin change interrupts
    CODE(53, N4Intr::enable_timer(POP()));          // TME - enable/disable timer2 interrupt
    CODE(54, NanoForth::call_api(POP()));           // API
#if N4_DOES_META
    ///> meta programming (for advance users)
    CODE(55, N4Asm::does(xt); xt = LFA_END);        // DO>
    CODE(56, N4Asm::create());          // CRE, create a word (header only)
    CODE(57, _nest(POP()));             // EXE  execute a given parameter field
    CODE(58, PUSH(N4Asm::query()));     // '    tick, get parameter field of a word
    CODE(59, N4Asm::comma(POP()));      // ,    comma, add a 16-bit value onto dictionary
    CODE(60, N4Asm::ccomma(POP()));     // C,   C-comma, add a 8-bit value onto dictionary
#endif // N4_DOES_META
    CODE(61, PUSH(*(vm.rp - 1)));       // I
    CODE(62, RPUSH(POP()));             // FOR
    CODE(63,                            // LIT
         DU v = FETCH(DIC(xt));         //   fetch the literal
         PUSH(v);                       //   put the value on TOS
         xt += sizeof(DU));             //   skip over the 16-bit literal
    }
    return xt;
}
///
///> opcode execution unit i.e. inner interpreter
///
void _nest(IU xt)
{
    RPUSH(LFA_END);                     // enter function call
    while (xt != LFA_END) {             ///> walk through instruction sequences
        U8 op  = *DIC(xt);              ///< fetch instruction
        U8 opx = op & CTL_BITS;         ///< masked opcode
#if TRC_LEVEL > 1        
        if (trc) N4Asm::trace(xt, op);  // execution tracing when enabled
#endif // TRC_LEVEL > 1
        if (opx==JMP_OPS) {             ///> branching opcodes?
            IU w = (((IU)op<<8) | *DIC(xt+1)) & ADR_MASK; // target address
            switch (op & JMP_MASK) {    // get branch opcode
            case OP_CALL:               // 0xc0 subroutine call
                serv_isr();             // loop-around every 256 ops
                RPUSH(xt + sizeof(IU)); // keep next instruction on return stack
                xt = w;                 // jump to subroutine till I_NOP
                break;
            case OP_CDJ: xt = POP() ? xt + sizeof(IU) : w; break;  // 0xd0 conditional jump
            case OP_UDJ: xt = w; break; // 0xe0 unconditional jump
            case OP_NXT:                // 0xf0 FOR...NXT
                if (!--(*(vm.rp-1))) {  // decrement counter *(rp-1)
                    xt += sizeof(IU);   // break loop
                    RPOP();             // pop off loop index
                }
                else xt = w;            // loop back
                serv_isr();             // loop-around every 256 ops
                break;
            }
        }
        else if (opx==PRM_OPS) {        ///> primitive word?
            xt = _invoke(op & PRM_MASK, ++xt); // advance one byte opcode
        }
        else {                          ///> handle number (1-byte literal)
            xt++;
            PUSH(op);                   // put the 7-bit literal on TOS
        }
    }
}
///
///> constructor and initializer
///
void setup(const char *code, Stream &io, U8 ucase)
{
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
    IU xt = N4Intr::isr();
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
    ok();                                   ///> console ok prompt if tib is empty
    U8  *tkn = get_token();                 ///> get a token from console
    U16 tmp;
    switch (N4Asm::parse(tkn, &tmp, 1)) {   ///> parse action from token (keep opcode in tmp)
    case TKN_IMM: _immediate(tmp);  break;  ///>> immediate words,
    case TKN_WRD: _nest(XT(tmp));   break;  ///>> execute colon word (user defined)
    case TKN_PRM: _invoke((U8)tmp); break;  ///>> execute primitive built-in word,
    case TKN_NUM: PUSH(tmp);        break;  ///>> push a number (literal) to stack top,
    case TKN_EXT:                           ///>> extended words, not implemented yet
    default:                                ///>> or, error (unknown action)
        show("?\n");
    }
}
} // namespace N4VM

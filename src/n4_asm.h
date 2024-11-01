/**
 * @file
 * @brief nanoForth Assembler class
 *
 * ####nanoForth Opcode formats
 *
 * @code
 *    branching : 11BB oooo oooo oooo            (12-bit absolute address)
 *    primitive : 10cc cccc                      (64 primitives)
 *    3-byte lit: 1011 1111 nnnn nnnn nnnn nnnn  bf xxxx xxxx (16-bit signed integer)
 *    1-byte lit: 0nnn nnnn                      (0..127)
 *    n-byte str: len, byte, byte, ...           (used in print str i.e. .")
 * @endcode
 */
#ifndef __SRC_N4_ASM_H
#define __SRC_N4_ASM_H
#include "n4.h"
///
/// @Note 1:
///   N4_DOES_META flag enable meta-programming,
///   it takes 6 opcodes slots i.e. 55~60
///   can be disabled if other opcodes need the slots
/// @Note 2:
///   N4_USE_GOTO flag use computed goto for primitive word invocation,
///   speeds up 5%, but takes extra 128 bytes of RAM,
///   can be disabled if some library needs extra memory 
///
#define N4_DOES_META  1 /**< enable meta programming */
#define N4_USE_GOTO   0 /**< use computed goto       */
///
/// parser actions enum used by execution and assembler units
///
enum N4OP {
    TKN_IMM = 0,     ///< immediate word
    TKN_WRD,         ///< colon (user defined) word
    TKN_PRM,         ///< primitive built-in word
    TKN_NUM,         ///< number (literal)
    TKN_EXT,         ///< extended built-in word
    TKN_ERR          ///< parse error (unknown token)
};
///
///@name Opcode Masks
///@{
constexpr U8  CTL_BITS = 0xc0;   ///< 1100 0000, JMP - 11nn xxxx, PRM - 10nn nnnn, NUM - 0nnn nnnn
constexpr U8  JMP_OPS  = 0xc0;   ///< 1100 0000
constexpr U8  PRM_OPS  = 0x80;   ///< 1000 0000
constexpr U8  JMP_MASK = 0xf0;   ///< 11nn xxxx, nn - CALL 00, CDJ 01, UDJ 10, NXT 11
constexpr U8  PRM_MASK = 0x3f;   ///< 00nn nnnn, 6-bit primitive opcodes
constexpr IU  ADR_MASK = 0x0fff; ///< 0000 aaaa aaaa aaaa 12-bit address in 16-bit branching instructions
///@}
///@name Opcode Prefixes
///@{
constexpr U8  OP_CALL  = 0xc0;   ///< 1100 0000
constexpr U8  OP_CDJ   = 0xd0;   ///< 1101 0000
constexpr U8  OP_UDJ   = 0xe0;   ///< 1110 0000
constexpr U8  OP_NXT   = 0xf0;   ///< 1111 0000
///@}
///
/// opcodes for loop control (in compiler mode)
/// Note: sequence in-sync with N4Asm::N4_WORDS, and N4VM::_invoke
///
enum N4_EXT_OP {                 ///< extended opcode (used by for...nxt loop)
    I_NOP  = 0,                  ///< NOP means EXIT
    I_SEM  = 10,                 ///< ; semi-colon
    I_DQ   = 31,                 ///< ." dot_string
    I_SQ   = 32,                 ///< S" do_string
    I_DO   = 55,                 ///< DO>
    I_I    = 61,                 ///< 61, loop counter
    I_FOR  = 62,                 ///< 62
    I_LIT  = 63                  ///< 63 = 0x3f 3-byte literal
};
constexpr IU LFA_END = 0xffff;   ///< end of link field
#define XT(a)  ((a) + sizeof(IU) + 3) /** adr + lnk[2] + name[3] */
/// LFA     NFA     XT=PFA
/// +-------+-------+--------------------+
/// |  lnk  | name  | parameters...I_RET |
/// +-------+-------+--------------------+
/// |       |       |
/// v       v       v
/// 16-bits 3-bytes variable length parameters
///
/// Assembler class
///
namespace N4Asm                     // (10-byte header)
{
    extern U8  *last;               ///< pointer to last word, for debugging
    extern U8  *here;               ///< top of dictionary (exposed to _vm for HRE, ALO opcodes)

    // EEPROM persistence I/O
    void save(U8 autorun=0);        ///< persist user dictionary to EEPROM
    IU   load(U8 autorun=0);        ///< restore user dictionary from EEPROM

    IU   reset();                   ///< reset internal pointers (for BYE)

    /// Instruction decoder
    N4OP parse(
        U8  *tkn,                   ///< token to be parsed
        IU  *rst,                   ///< parsed result
        U8  run                     ///< run mode flag (1: run mode, 0: compile mode)
        );
    /// Forth compiler
    void compile(
        IU *rp0                     ///< memory address to be used as assembler return stack
        );
    void variable();                ///< create a variable on dictionary
    void constant(DU v);            ///< create a constant on dictionary
    /// meta compiler
    void create();                  ///< create a word name field
    void comma(DU v);               ///< compile a 16-bit value onto dictionary
    void ccomma(DU v);              ///< compile a 8-it value onto dictionary
    void does(IU xt);               ///< metaprogrammer (jump to definding word DO> section)
    void dot_str();
    /// dictionary, string list scanners
    IU  query();                    ///< get xt of next input token, 0 if not found
    void words();                   ///< display words in dictionary
    void forget();                  ///< forgets word in the dictionary
    void see();                     ///< decode colon word

    /// print execution tracing info
    U16 trace(
        IU   adr,                   ///< address to word to be executed
        U8   ir,                    ///< instruction register value
        char delim=0                ///< token delimiter
        );
};  // namespace N4Asm

#endif //__SRC_N4_ASM_H

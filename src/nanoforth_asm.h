/**
 * @file nanoforth_asm.h
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
#ifndef __SRC_NANOFORTH_ASM_H
#define __SRC_NANOFORTH_ASM_H
#include "nanoforth_core.h"
///
/// parser actions enum used by execution and assembler units
///
enum N4OP {
    TKN_IMM = 1,     ///< immediate word
    TKN_WRD,         ///< colon (user defined) word
    TKN_EXT,         ///< extended built-in word
    TKN_PRM,         ///< primitive built-in word
    TKN_NUM,         ///< number (literal)
    TKN_ERR          ///< parse error (unknown token)
};
///
///@name Opcode Masks
///@{
constexpr U8  CTL_BITS = 0xc0;   ///< 1100 0000, JMP - 11nn xxxx, PRM - 10nn nnnn, NUM - 0nnn nnnn
constexpr U8  JMP_OPS  = 0xc0;   ///< 1100 0000
constexpr U8  JMP_MASK = 0xf0;   ///< 11nn xxxx, nn - CALL 00, CDJ 01, UDJ 10, RET 11
constexpr U8  PRM_OPS  = 0x80;   ///< 1000 0000
constexpr U8  PRM_MASK = 0x3f;   ///< 00nn nnnn, 6-bit primitive opcodes
constexpr U16 ADR_MASK = 0x0fff; ///< 0000 aaaa aaaa aaaa 12-bit address in 16-bit branching instructions
///@}
///@name Opcode Prefixes
///@{
constexpr U8  OP_CALL  = 0xc0;   ///< 1100 0000
constexpr U8  OP_CDJ   = 0xd0;   ///< 1101 0000
constexpr U8  OP_UDJ   = 0xe0;   ///< 1110 0000
constexpr U8  OP_RET   = 0xf0;   ///< 1111 0000
///@}
///
/// opcodes for loop control (in compiler mode)
///
enum N4_EXT_OP {                 ///< extended opcode (used by for...nxt loop)
    I_DQ   = 29,                 ///< ." handler (adjust, if field name list changed)
    I_I    = 60,                 ///< 60 loop counter
    I_FOR,                       ///< 61
    I_NXT,                       ///< 62
    I_LIT                        ///< 63 = 0x3f 3-byte literal
};
constexpr U16 LFA_X    = 0xffff;     ///< end of link field
///
/// Assembler class
///
class N4Asm : N4Core                // (10-byte header)
{
    U8  *dic;                       ///< dictionary base
    U16 *rp;                        ///< return stack pointer

    U8  tab;                        ///< tracing indentation counter
    U8  xxx;                        ///< reserved

public:
    U8  *last;                      ///< pointer to last word, for debugging
    U8  *here;                      ///< top of dictionary (exposed to _vm for HRE, ALO opcodes)

    /// Assembler constructor
    N4Asm(
        U8 *mem                     ///< pointer of memory block for dictionary
        );
    U16 reset();                    ///< reset internal pointers (for BYE)

    /// Instruction Decoder
    N4OP parse_token(
        U8 *tkn,                    ///< token to be parsed
        U16 *rst,                   ///< parsed result
        U8 run                      ///< run mode flag (1: run mode, 0: compile mode)
        );

    /// Forth compiler
    void compile(
        U16 *rp0                    ///< memory address to be used as assembler return stack
        );
    void variable();                ///< create a variable on dictionary
    void constant(S16 v);           ///< create a constant on dictionary

    // dictionary, string list scanners
    /// query(token) in dictionary for existing word
    U8   query(
        U8 *tkn,                    ///< token to be searched
        U16 *adr                    ///< function address of the found word
        );
    void words();                   ///< display words in dictionary
    void forget();                  ///< forgets word in the dictionary

    // EEPROM persistence I/O
    void save(bool autorun=false);  ///< persist user dictionary to EEPROM
    U16  load(bool autorun=false);  ///< restore user dictionary from EEPROM

    // execution tracing
    /// print execution tracing info
    void trace(
        U16 adr,                    ///< address to word to be executed
        U8 ir                       ///< instruction register value
        );

private:
    void _add_word();               ///< create name field and link to previous word
    void _add_branch(U8 op);        ///< manage branching opcodes
    void _add_str();                ///< add string for ."
    void _list_voc(U16 n);          ///< list words from all vocabularies
};
#endif //__SRC_NANOFORTH_ASM_H

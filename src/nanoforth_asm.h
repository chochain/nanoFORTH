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
    TKN_DIC,         ///< dictionary (user defined) word
    TKN_EXT,         ///< extended built-in word
    TKN_PRM,         ///< primitive built-in word
    TKN_NUM,         ///< number (literal)
    TKN_ERR          ///< parse error (unknown token)
};
///
///@name Opcode Masks
///@{
constexpr U8  CTL_BITS = 0xc0;   ///< 1100 0000, 11xx: JMP, 10xx: PRM, 0xxx: NUM
constexpr U8  PFX_PRM  = 0x80;   ///< 1000 0000
constexpr U8  PRM_MASK = 0x3f;   ///< 0011 1111, 6-bit primitive opcodes
constexpr U8  JMP_MASK = 0xf0;   ///< 1111 0000
constexpr U16 ADR_MASK = 0x0fff; ///< 0000 aaaa aaaa aaaa 12-bit address in 16-bit branching instructions
///@}
///@name Opcode Prefixies
///@{
constexpr U8  PFX_CALL = 0xc0;   ///< 1100 0000
constexpr U8  PFX_CDJ  = 0xd0;   ///< 1101 0000
constexpr U8  PFX_UDJ  = 0xe0;   ///< 1110 0000
constexpr U8  PFX_RET  = 0xf0;   ///< 1111 0000
///@}
///
/// opcodes for loop control (in compiler mode)
///
enum N4_EXT_OP {                 ///< extended opcode (used by for...nxt loop)
    I_DQ   = 0x1d,               ///< ." handler (adjust, if field name list changed)
    I_FOR  = 0x3b,               ///< 0x3b
    I_NXT,                       ///< 0x3c
    I_BRK,                       ///< 0x3d
    I_I,                         ///< 0x3e loop counter
    I_LIT                        ///< 0x3f 3-byte literal
};
///
/// Assembler class
///
class N4Asm : N4Core                // (10-byte header)
{
    U8  *dic;                       ///< dictionary base
    U16 *rp;                        ///< return stack pointer
    
    U8  autorun;                    ///< auto load and run from EEPROM
    U8  tab;                        ///< tracing indentation counter
    
public:
    U8  *last;                      ///< pointer to last word, for debugging
    U8  *here;                      ///< top of dictionary (exposed to _vm for HRE, ALO opcodes)
    
    /// Assembler constructor
    N4Asm(                          
        U8 *mem                     ///< pointer of memory block for dictionary
        );                 
    void reset();                   ///< reset internal pointers (for BYE)
    
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
        U16 *adr                    ///< function addreess of the found word
        );      
    void words();                   ///< display words in dictionary
    void forget();                  ///< forgets word in the dictionary

    // EEPROM persistence I/O
    void save();                    ///< persist user dictionary to EEPROM
    void load();                    ///< restore user dictionary from EEPROM

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
    void _list_voc();               ///< list words from all vocabularies
};    
#endif //__SRC_NANOFORTH_ASM_H

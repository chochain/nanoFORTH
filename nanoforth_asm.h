///
/// \file nanoforth_asm.h
/// \brief NanoForth Assembler class
///
///> NanoForth Opcode formats<br>
///>>    `1-byte lit: 0nnn nnnn                      (0..127)`<br>
///>>    `3-byte lit: 1011 1111 nnnn nnnn nnnn nnnn  bf xxxx xxxx (16-bit signed integer)`<br>
///>>    `primitive : 10cc cccc                      (64 primitives)`<br>
///>>    `branching : 11BB oooo oooo oooo            (12-bit absolute address)`<br>
///
#ifndef __SRC_NANOFORTH_ASM_H
#define __SRC_NANOFORTH_ASM_H
#include "nanoforth.h"
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
/// branch flags   (01BB)
///
#define CTL_BITS   0xc0       /**< 1100 0000, 11xx: JMP, 10xx: PRM, 0xxx: NUM */
#define PFX_PRM    0x80       /**< 1000 0000 */
#define PRM_MASK   0x3f       /**< 0011 1111, 6-bit primitive opcodes */
#define JMP_MASK   0xf0       /**< 1111 0000 */
#define PFX_UDJ    0xc0       /**< 1100 0000 */
#define PFX_CDJ    0xd0       /**< 1101 0000 */
#define PFX_CALL   0xe0       /**< 1110 0000 */
#define PFX_RET    0xf0       /**< 1111 0000 */
#define ADR_MASK   0x0fff     /**< 0000 aaaa aaaa aaaa 12-bit address */
///
/// opcodes for loop control (in compiler mode)
///
enum N4_EXT_OP {              ///< extended opcode (used by for...nxt loop)
	I_DQ   = 0x19,            ///< ." handler (adjust, if field name list changed)
    I_FOR  = 0x3b,            ///< 0x3b
    I_NXT,                    ///< 0x3c
    I_BRK,                    ///< 0x3d
    I_I,                      ///< 0x3e loop counter
    I_LIT                     ///< 0x3f 3-byte literal
};
///
/// NanoForth Assembler class
///
class N4Asm                         // (10-byte header)
{
    U8  *dic;                       ///< dictionary base
    U16 *rp;                        ///< return stack pointer
    
    U8  tab;                        ///< tracing indentation counter
    U8  xxx;                        ///< reserved          
    
public:
    U8  *last;                      ///< pointer to last word (exposed to _vm::_extended for debugging)
    U8  *here;                      ///< top of dictionary    (exposed to _vm::_extended for debugging)
    
    N4Asm();                        ///< NanoForth Assembler object constructor
    void init(U8 *mem);             ///< intializer (Arduino does not have dynamic constructor)
    void reset();                   ///< reset internal pointers (for BYE)

    /// NanoForth Instruction Decoder
    N4OP parse_token(U8 *tkn, U16 *rst, U8 run); 

    /// proxy to NanoForth assembler
    void compile(U16 *rp0);             ///< create word on dictionary
    void variable();                    ///< create variable on dictionary

    // dictionary, string list scanners
    void words();                       ///< display words in dictionary
    U8   query(U8 *tkn, U16 *adr);      ///< query(token) in dictionary for existing word
    void forget();                      ///< forgets word in the dictionary

    // EEPROM persistance I/O
    void save();                        ///< persist user dictionary to EEPROM
    void load();                        ///< restore user dictionary from EEPROM

    // execution tracing
    void trace(U16 a, U8 ir);           ///< print execution tracing info
    
private:
    void _list_voc();                                ///< list words from all vocabularies
    void _do_branch(U8 op);                          ///< manage branching opcodes
    void _opname(U8 op, const char *lst, U8 space);  ///< display opcode 3-char name
    void _do_str();                                  ///< add string for ."
};    
#endif //__SRC_NANOFORTH_ASM_H

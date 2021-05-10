///
/// \file nanoforth_asm.h
/// \brief NanoForth Assembler class
///
///> NanoForth Opcode formats<p>
///>>    `primitive : 111c cccc                      (32 primitive)`<p>
///>>    `branching : 1BBo oooo oooo oooo            (+- 12-bit relative offset, i.e. portable)`<p>
///>>    `1-byte lit: 0nnn nnnn                      (0..127)`<p>
///>>    `3-byte lit: 1111 1110 nnnn nnnn nnnn nnnn  FE xxxx xxxx (16-bit signed integer)`<p>
///
#ifndef __SRC_NANOFORTH_ASM_H
#define __SRC_NANOFORTH_ASM_H
#include "nanoforth.h"
///
/// \enum
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
/// branch flags (1BBx)
///
#define JMP_BIT    0x1000     /**< 0001 0000 0000 0000 12-bit offset */
#define PFX_UDJ    0x80       /**< 1000 0000 */
#define PFX_CDJ    0xa0       /**< 1010 0000 */
#define PFX_CALL   0xc0       /**< 1100 0000 */
#define PFX_PRM    0xe0       /**< 1110 0000 */
///
/// \enum opcodes for loop control (in compiler mode)
///
enum {
    I_LOOP = (PFX_PRM | 25),  ///< f 9
    I_I,                      ///< f a
    I_RD2,                    ///< f b
    I_P2R2,                   ///< f c
    I_RET,                    ///< f d
    ///
    /// extended opcode and literal (in compiler mode)
    ///
    I_LIT,                    ///< f e
    I_EXT                     ///< f f
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
    void trace(U16 a, U8 ir, U8 *pc);   ///< print execution tracing info
    
private:
    void _list_voc();                                ///< list words from all vocabularies
    void _do_branch(U8 op);                          ///< manage branching opcodes
    void _opname(U8 op, const char *lst, U8 space);  ///< display opcode 3-char name
};    
#endif //__SRC_NANOFORTH_ASM_H

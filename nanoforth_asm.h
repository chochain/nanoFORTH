#ifndef __SRC_NANOFORTH_ASM_H
#define __SRC_NANOFORTH_ASM_H
#include "nanoforth.h"

// ============================================================================================
// Opcode formats
//     primitive:  111c cccc                      (32 primitive)
//     branching:  1BBo oooo oooo oooo            (+- 12-bit relative offset, i.e. portable)
//     1-byte lit: 0nnn nnnn                      (0..127)
//     3-byte lit: 1111 1110 nnnn nnnn nnnn nnnn  FE xxxx xxxx (16-bit signed integer)
// ============================================================================================
//
// branch flags (1BBx)
//
#define JMP_BIT    0x1000     /* 0001 0000 0000 0000 12-bit offset */
#define PFX_UDJ    0x80       /* 1000 0000 */
#define PFX_CDJ    0xa0       /* 1010 0000 */
#define PFX_CALL   0xc0       /* 1100 0000 */
#define PFX_PRM    0xe0       /* 1110 0000 */

enum {
    //
    // opcodes for loop control (in compiler mode)
    //
    I_LOOP = (PFX_PRM | 25),  /* f 9 */ 
    I_I,                      /* f a */
    I_RD2,                    /* f b */
    I_P2R2,                   /* f c */
    I_RET,                    /* f d */
    //
    // extended opcode and literal (in compiler mode)
    //
    I_LIT,                   /* f e */
    I_EXT                    /* f f */
};

class N4Asm {
    U8  *dic;                       // dictionary base
    U16 *rp;                        // return stack pointer
    
    U8  tab;                        // tracing indentation counter
    
public:
    U8  *last;                      // pointer to last word
    U8  *here;                      // top of dictionary
    
    N4Asm(U8 *mem, U16 mem_sz);
    void reset();                   // reset internal pointers (for BYE)
    //
    // assembler functions
    //
    U8   parse_token(U8 *tkn, U16 *rst, U8 run);
    void compile();                // create word on dictionary
    void variable();               // create variable on dictionary
    //
    // dictionary, string list scanners
    //
    void words();                  // display words in dictionary
    U8   query(U8 *tkn, U16 *adr); // query(token) in dictionary for existing word
    void forget();                 // forgets word in the dictionary
    //
    // EEPROM persistance I/O
    //
    void save();
    void load();
    //
    // execution tracing
    //
    void trace(U16 a, U8 ir, U8 *pc);
    
private:
    void _list_voc();              // list words from all vocabularies
    void _do_branch(U8 op);        // manage branching opcodes
    void _opname(U8 op, const char *lst, U8 space);
};    
#endif //__SRC_NANOFORTH_ASM_H

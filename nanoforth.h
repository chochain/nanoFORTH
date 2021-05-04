#ifndef __SRC_NANOFORTH_H
#define __SRC_NANOFORTH_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <Arduino.h>

#define ASM_TRACE    1                /* enable assembler tracing    */
#define EXE_TRACE    1                /* enable VM execution tracing */

typedef uint8_t      U8;
typedef uint16_t     U16;
typedef int16_t      S16;
typedef uint32_t     U32;

#define TIB_SZ       0x40
#define STK_SZ       0x80
#define DIC_SZ       0x400
#define MEM_SZ       (DIC_SZ+STK_SZ)
//
// parser flag
//
enum {
    TKN_EXE = 1,
    TKN_DIC,
    TKN_EXT,
    TKN_PRM,
    TKN_NUM,
    TKN_ERR
};
// ============================================================================================
// Opcode formats
//     primitive:  111c cccc                      (32 primitive)
//     branching:  1BBo oooo oooo oooo            (+- 12-bit relative offset, i.e. portable)
//     1-byte lit: 0nnn nnnn                      (0..127)
//     3-byte lit: 1111 1111 nnnn nnnn nnnn nnnn
// ============================================================================================
//
// branch flags (1BBx)
//
#define JMP_BIT    0x1000          /* 0001 0000 0000 0000 12-bit offset */
#define PFX_UDJ    0x80            /* 1000 0000 */
#define PFX_CDJ    0xa0            /* 1010 0000 */
#define PFX_CALL   0xc0            /* 1100 0000 */
#define PFX_PRM    0xe0            /* 1110 0000 */
//
// opcodes for loop control (in compiler mode)
//
#define I_LOOP     (PFX_PRM | 25)  /* f 9 */ 
#define I_RD2      (PFX_PRM | 26)  /* f a */
#define I_I        (PFX_PRM | 27)  /* f b */
#define I_P2R2     (PFX_PRM | 28)  /* f c */
#define I_RET      (PFX_PRM | 29)  /* f d */
//
// extended opcode and literal (in compiler mode)
//
#define I_LIT      (PFX_PRM | 30)  /* f e */
#define I_EXT      (PFX_PRM | 31)  /* f f */
//
// global variables
//
extern U8   *dic, *here, *last;    // dictionary pointers
extern U16  *rp;                   // return stack pointer
extern S16  *sp;                   // parameter stack pointer
//
// dictionary index <=> pointer translation macros
//
#define PTR(n)     ((U8*)dic + (n))
#define IDX(p)     ((U16)((U8*)(p) - dic))
//
// Forth stack opcode macros
//
#define TOS        (*sp)
#define TOS1       (*(sp+1))
#define PUSH(v)    (*(--sp)=(S16)(v))
#define POP()      (*(sp++))
#define RPUSH(v)   (*(rp++)=(U16)(v))
#define RPOP()     (*(--rp))
//
// memory access opcodes
//
#define SET8(p, c)  (*(U8*)(p)++=(U8)(c))
#define SET16(p, n) do { U16 x=(U16)(n); SET8(p,(x)>>8); SET8(p,(x)&0xff); } while(0)
#define GET16(p)    (((U16)(*(U8*)(p))<<8) + *((U8*)(p)+1))
//
// tracing functions
//
void d_chr(char c);
void d_hex(U8 c);
void d_adr(U16 a);
void d_ptr(U8 *p);
//
// IO and Search Functions =================================================
//
#define putstr(msg)    Serial.print(F(msg))
#define putchr(c)      Serial.write((char)c)
#define puthex(v)      Serial.print((U16)v, HEX)
void putnum(S16 n);                             // sent a number literal to console
U8   getnum(U8 *str, S16 *num);                 // process a literal
U8   *token(void);                              // get a token from console input
//
// memory dummpers
//
void memdump(U8 *p0, U8 *p1, U8 d);             // show memory content between pointers (d: delimiter option)
void dump(U16 idx, U16 sz);                     // dump memory block with dictionary offset and length
//
// dictionary, string list scanners
//
U8   query(U8 *tkn, U16 *adr);                    // query(token) in dictionary for existing word
U8   find(U8 *tkn, const char *lst, U16 *id);     // find(token) in string list
U8   parse_token(U8 *tkn, U16 *rst, U8 run);      // parse and action
//
// Forth VM core functions
//
void vm_setup();
void vm_core();
void compile();             // create word on dictionary
void variable();            // create variable on dictionary
void extended(U8 op);       // additional primitive words

U8   vm_getchar();
void vm_delay(U32 ms);

#endif // __SRC_NANOFORTH_H

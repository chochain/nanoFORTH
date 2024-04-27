/*
  Tiny FORTH
  System-dependent module for MinGW

  T. Nakagawa

  2004/07/10
*/


#ifndef _SYSTEM_H_
#define _SYSTEM_H_


#define BUF_SIZE 128	/* 8 - 255 */
#define STACK_SIZE (1024 * 8)	/* 8 - 65536 */
#define DIC_SIZE (1024 * 8)	/* 8 - 8*1024 */
#define R2V(base, ptr) ((unsigned short)((ptr) - (base)))
#define V2R(base, ptr) ((unsigned char *)((base) + (ptr)))


void initl(void);
unsigned char getchr(void);
void putchr(unsigned char c);


#endif

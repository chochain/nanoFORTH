/*
  Tiny FORTH
  System-dependent module for H8/3694F

  T. Nakagawa

  2004/08/05
*/


#include "system.h"
#include "3694S.H"


void initl(void) {
  volatile int w;

  /* SCI */
  SCI3.SCR3.BYTE = 0;
  SCI3.SMR.BYTE = 0;
  SCI3.BRR = 32;	/* 19200bps @ 20MHz */
  for (w = 0; w < 1000; w++) ;
  IO.PMR1.BIT.TXD = 1;
  SCI3.SCR3.BIT.RE = 1;
  SCI3.SCR3.BIT.TE = 1;

  return;
}


unsigned char getchr(void) {
  unsigned char c;
  while ((SCI3.SSR.BYTE & 0x78) == 0) ;
  if (SCI3.SSR.BIT.RDRF == 1) {
    c = SCI3.RDR;
  } else {
    SCI3.SSR.BYTE &= 0x84;
    c = 0;
  }
  putchr(c);
  return c;
}


void putchr(unsigned char c) {
  while (SCI3.SSR.BIT.TDRE == 0) ;
  SCI3.TDR = c;
  return;
}

/*
  Tiny FORTH
  System-dependent module for MinGW

  T. Nakagawa

  2004/07/10
*/


#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "system.h"


void initl(void) {
  return;
}


unsigned char getchr(void) {
  int c;
  c = getch();
  if (c == '\x03') exit(0);	/* CTRL+C */
  if (c < 0) c = 0;
  putch(c);
  return (unsigned char)c;
}


void putchr(unsigned char c) {
  putch(c);
  return;
}

/*
  Tiny FORTH
  System-dependent module for Linux/FreeBSD/Cygwin

  T. Nakagawa

  2004/07/10
*/


#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "system.h"
#include "conio.h"


void initl(void) {
  signal(SIGINT, exit);
  signal(SIGTERM, exit);

  return;
}


unsigned char getchr(void) {
  int c;
  c = getch();
  if (c < 0) c = 0;
  putch(c);
  return (unsigned char)c;
}


void putchr(unsigned char c) {
  putch(c);
  return;
}

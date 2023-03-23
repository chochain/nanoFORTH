/**
 * @file
 * @brief nanoForth Arduino interface
 */
#ifndef __SRC_NANOFORTH_H
#define __SRC_NANOFORTH_H

extern void n4_setup(const char *code=0, Stream &io=Serial, int ucase=0);
extern void n4_api(int i, void (*fp)());
extern void n4_push(int v);
extern int  n4_pop();
extern void n4_run();

#endif // __SRC_NANOFORTH_H

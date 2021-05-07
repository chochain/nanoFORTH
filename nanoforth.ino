/*
 nanoFORTH - Forth for Arduino Nano (and UNO)
 
 2004-07 T. NAKAGAWA (Tiny FORTH), original reference 
 2020-08 circuit4u@medium.com (Tiny Forth NXT), Arduino reference
 2021-03 CC Lee: modularize with comments [6698,1006]; add buffer handler[6924,1036]
 2021-04-02: CC - add multi-tasker, byte count[6782/1056],
    [6930,1056] add threads
    [6908,1014] use F()
    [6860,852]  use PROGMEM, pgm_read_byte in find()
    [6906,852]  getnum support negative number
    [7012,860]  add digitalR/W
  2021-04-03: CC
    [6898,852]  add vm_delay
    [6912,852]  update EEPROM IO
    [6842,852]  execution tracing enable/disable
    [6922,852]  add CELL (CEL), ALLOT (ALO) opcodes
  2021-04-16: CC
    [7446,932]  add list_word, use 80-byte console buffer
    [7396,932]  add ASM_TRACE from EXE_TRACE options
    [7676,802]  grow Task at end of heap
    [8266,802]  if use array instead of pointer arithmetics, revert!
  2021-05-03: CC
    [7818,1428] forget Task struct; grow MEM_SZ to DIC_SZ+STK_SZ (1K+64*2)
    [8254,1430] add execution tracing option
  2021-05-06: CC
    [9214,1424] refactor to C++ for Arduino lib
*/
#include "nanoforth.h"

NanoForth n4;                 // create a nanoForth instance

void setup()
{
    Serial.begin(115200);
}

void loop()
{
    n4.run();                  // execute one vm cycle
}

void n4_loop()
{    
    digitalWrite(LED_BUILTIN, HIGH);
    n4_delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    n4_delay(500);
}


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
*/
#include <pt.h>
#include "nanoforth.h"
//
// thread handler (note: using macro to make it stackless)
//
#define PT_DELAY_msec(th, ms)  do {    \
    static U32 t;                      \
    t = millis() + (U32)(ms);          \
    PT_WAIT_UNTIL(th, millis()>=t);    \
} while(0)

static struct pt ctx_hw;                    // protothread contexts
PT_THREAD(hw_thread())                      // hardward protothread
{
    PT_BEGIN(&ctx_hw);
    /*
    U16 tmp;
    find("LD ", LST_EXT, &tmp);             // Load DIC
    extended((U8)tmp);
    
    if (lookup("INI", &tmp)) {              // RUN "INI"
        execute(tmp + 2 + 3);               // header: 2-byte pointer to next + 3-byte NAME
    }  
    */
    while (1) {
        digitalWrite(LED_BUILTIN, HIGH);
        PT_DELAY_msec(&ctx_hw, 500);
        digitalWrite(LED_BUILTIN, LOW);
        PT_DELAY_msec(&ctx_hw, 200);
    }
    PT_END(&ctx_hw);
}
#define YIELD()      PT_SCHEDULE(hw_thread())
//
// console input with cooperative threading
//
U8 vm_getchar()
{
    while (!Serial.available()) YIELD();   // steal cycles for hardware stuffs
    return (U8)Serial.read();
}

void vm_delay(U32 ms)
{
    U32 t = millis() + ms;
    while (millis()<t) YIELD();            // run hardware cycles while waiting
}
const char CMD[] PROGMEM = "\x05" \
    ":  " "VAR" "FGT" "DMP" "BYE";
const char JMP[] PROGMEM = "\x0b" \
    ";  " "IF " "ELS" "THN" "BGN" "UTL" "WHL" "RPT" "DO " "LOP" \
    "I  ";
const char PRM[] PROGMEM = "\x19" \
    "DRP" "DUP" "SWP" ">R " "R> " "+  " "-  " "*  " "/  " "MOD" \
    "AND" "OR " "XOR" "=  " "<  " ">  " "<= " ">= " "<> " "NOT" \
    "@  " "!  " "C@ " "C! " ".  ";
const char EXT[] PROGMEM = "\x0e" \
    "HRE" "CP " "OVR" "INV" "CEL" "ALO" "WRD" "SAV" "LD " "DLY" \
    "IN " "OUT" "AIN" "TRC";

void _opname(U8 op, const char *lst, U8 space)
{
    PGM_P p = reinterpret_cast<PGM_P>(lst)+1+op*3;
    char  c;
    d_chr(pgm_read_byte(p));
    if ((c=pgm_read_byte(p+1))!=' ' || space) d_chr(c);
    if ((c=pgm_read_byte(p+2))!=' ' || space) d_chr(c);
}

void list_words()
{
    const char *lst[] PROGMEM = { CMD, JMP, PRM, EXT };
    U8 n = 0;
    for (U8 i=0; i<4; i++) {
        U8 sz = pgm_read_byte(reinterpret_cast<PGM_P>(lst[i]));
        for (U8 op=0; op<sz; op++) {
            d_chr(n++%10==0 ? '\n' : ' ');
            _opname(op, lst[i], 1);
        }
        YIELD();
    }
    d_chr('\n');
}

U8 parse_token(U8 *tkn, U16 *rst, U8 run)
{
    if (find(tkn, run ? CMD : JMP, rst)) return TKN_EXE; // run, compile mode
    if (query(tkn, rst))                 return TKN_DIC; // search word dictionary addr(2), name(3)
    if (find(tkn, EXT, rst))             return TKN_EXT; // search extended words
    if (find(tkn, PRM, rst))             return TKN_PRM; // search primitives
    if (getnum(tkn, (S16*)rst))          return TKN_NUM; // parse as number literal
    
    return TKN_ERR;
}
//
// Execution tracer
//
U8 tab, trc;
void vm_trace_set(U16 f)
{
    trc += f ? 1 : (trc ? -1 : 0);
    tab = 0;
}

void _indent(U8 n)
{
    putstr("\n....");
    for (int i=0; i<n; i++) {                         // indentation per call-depth
        putstr("  ");
    }
}

const char PMX[] PROGMEM = " LOPI  RD2DO ";
void vm_trace(U16 a, U8 ir, U8 *pc)
{
    if (!trc) return;
    
    U8 spc=1;
    d_adr(a);                                         // tracing info
    
    if ((ir & 0x80)==0) { d_chr('#'); d_hex(ir);             }    // 1-byte literal
    else if (ir==I_LIT) { d_chr('#'); putnum(GET16(pc));     }    // 3-byte literal
    else if (ir==I_RET) { d_chr(';'); _indent(tab-=tab?1:0); spc=0; }    // RET
    else if (ir==I_EXT) { d_chr('_'); _opname(*pc, EXT, 0);  }    // EXT extended words
    else {
        U8 op = ir & 0x1f;                            // opcode or top 5-bit of offset
        a += ((U16)op<<8) + *pc - JMP_BIT;            // JMP_BIT ensure 2's complement (for backward jump)
        switch (ir & 0xe0) {
        case PFX_UDJ: d_chr('j'); d_adr(a);         break;        // 0x80 unconditional jump
        case PFX_CDJ: d_chr('?'); d_adr(a);         break;        // 0xa0 conditional jump
        case PFX_CALL:                                            // 0xc0 word call
            d_chr(':');
            pc = PTR(a)-3;                                        // backtrack 3-byte (name field)
            d_chr(*pc++); d_chr(*pc++); d_chr(*pc);
            _indent(++tab);
            spc = 0;
            break;
        case PFX_PRM:
            d_chr('_');
            _opname(op<25 ? op : op-25, op<25 ? PRM : PMX, 0);
            break;
        }
    }
    if (spc) d_chr(' ');
}

void setup()
{
    Serial.begin(115200);
    PT_INIT(&ctx_hw);          // initialize hardware thread
    vm_setup();                // setup Forth virtual machine

    tab = trc = 0;             // tracing flags
    //
    // show system info
    //
    putstr("MEM_SZ=x");  puthex(MEM_SZ);
    putstr(", DIC_SZ=x");  puthex(MEM_SZ-STK_SZ);
    putstr(", STK_SZ=x");  puthex(STK_SZ);
    putstr(", TIB_SZ=x");  puthex(TIB_SZ);
    putstr(" ");
}

void loop()
{
    vm_core();                 // execute one vm cycle
    YIELD();                   // give hardware some CPU time
}


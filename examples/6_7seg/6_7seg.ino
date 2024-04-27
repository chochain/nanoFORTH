/**
 *  @file
 *  @brief nanoFORTH example - 4 digit 7-segment display
 *
 *  Assuming you have a 12-pin 4 digit 7-segment LED hooked up as following
 *
 *      A             D4 A  F  D3 D2 B
 *     ---       +----^--^--^--^--^--^---+   
 *  F |   | B    |     |     |     |     |
 *     -G-       |  8  |  8  |  8  |  8  |
 *  E |   | C    |     |     |     |     |
 *     ---       +----v--v--v--v--v--v---+
 *      D  *DP        E  D  DP C  G  D1
 *
 *  + A,B,C,D,E,F,G,DP => 4,5,6,7,8,9,10,11
 *  + D1,D2,D3,D4      => A0,A1,A2,A3       i.e. digital pin 14,15,16,17
 *  + or check this Wokwi project https://wokwi.com/projects/358961171560345601
 *
 *  How To:
 *  Open Serial Monitor (or your favorate terminal emulator) as the console input to nanoFORTH
 *  + baud rate set to 115200
 *  + line ending set to Both NL & CR (if using emulator, set Add CR on, ECHO on)
 *  + type the following Forth code into Serial Monitor input
 */
#include <nanoFORTH.h>

const char code[] PROGMEM =
// "1 TRC\n"                                           // turn tracing on for debugging
": ini 16 FOR 1 I 1 + PIN NXT ; ini FGT ini\n"         // set pin 2~17 to OUTPUT
"CRE x $F360 , $B5F4 , $66D6 , $D770 , $F776 ,\n"      // x keeps 7-seg pin patterns of 0~9, 1-byte each
"CRE vx 4 ALO\n"                                       // vx cache pin patterns for 4 digits
": vx! 1 - vx + C! ;\n"                                // ( n i -- ) vx[i] = (char)n
": v! 4 FOR DUP 10 MOD x + C@ I vx! 10 / NXT DRP ;\n"  // ( n -- ) n => pin patterns digit-by-digit
"CRE d $0E0D , $0B07 ,\n"                              // d keeps 4 digit control (aaaA, aaAa, aAaa, Aaaa)
": sd d + C@ $30F OUT ;\n"                             // ( i -- ) select output digit to port C (A0~A6)
"VAR i 0 i !\n"                                        // i controls the i'th digit to display
": 7s DUP sd vx + C@ DUP $1F0 OUT $20F OUT ;\n"        // ( i -- ) send vx[i] to port D (0~7) & B (8~13)
": dsp i @ 1 + 3 AND DUP i ! 7s ;\n"                   // ( -- ) cycle i to display the i'th digit
// "1234 v!\n"                                         // incremental test number
// "0 dsp 500 DLY 1 dsp 500 DLY 2 dsp 500 DLY 3 dsp\n" // tests 4-digit display
"5 0 TMI dsp\n"                                        // timer interrupt[0] to update display every 5ms
"VAR cnt 1000 cnt !\n"                                 // cnt is a counter we want it value on display
": c++ cnt @ 1 + DUP cnt ! v! ;\n"                     // ( -- ) increment the counter and cache patterns
"100 1 TMI c++\n"                                      // timer interrupt[1] to update c every 100ms
".\" type 1 TME to start\"\n"
;

void setup() {
    Serial.begin(115200);            ///< init Serial Monitor
    n4_setup(code);                  /// * setup nanoFORTH with preload Forth code
}

void loop() {
    n4_run();                        ///< execute VM of our NanoForth instance
}

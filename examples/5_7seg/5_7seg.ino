/**
 *  @file
 *  @brief nanoFORTH example - 4 digit 7-segment display
 *
 *  Assuming you have a 12-pin 4 digit 7-segment LED hooked up as following
 *  + A,B,C,D,E,F,G,DP => 4,5,6,7,8,9,10,11
 *  + D1,D2,D3,D4      => 2,3,12,13
 *  + or check this Wokwi project https://wokwi.com/projects/358961171560345601
 *
 *  How To:
 *  Open Serial Monitor (or your favorate terminal emulator) as the console input to nanoFORTH
 *  + baud rate set to 115200
 *  + line ending set to Both NL & CR (if using emulator, set Add CR on, ECHO on)
 *  + type the following Forth code into Serial Monitor input
 *    > WRD
 *    > 1 TME
 */
#include <nanoFORTH.h>

const char code[] PROGMEM =
": ini 12 FOR 1 I 1 + PIN NXT ; ini FGT ini\n"         // set pin 2~13 to OUTPUT
"VAR x 8 ALO\n"                                        // x keeps 7-seg pin patterns of 0~9
"$F360 x ! $B5F4 x 2 + ! $66D6 x 4 + !\n"
"$D770 x 6 + ! $F776 x 8 + !\n"
"VAR d 2 ALO $3834 d ! $2C1C d 2 + !\n"                // d keeps digit control pin patterns
": 7d d + C@ DUP $10C OUT $230 OUT ;\n"                // ( n -- ) set output digit
"VAR vx 2 ALO\n"                                       // vx cache pattern for 4 digits before display
": vx! 1 - vx + C! ;\n"                                // ( i -- ) update vx by index
": ?v 4 FOR DUP 10 MOD x + C@ I vx! 10 / NXT DRP ;\n"  // ( n -- ) process number into 4-digit bit patterns
": 7s vx + C@ DUP $1F0 OUT $20F OUT ;\n"               // ( i -- ) display i'th digit on 7-seg
"VAR i 0 i !\n"                                        // i is the 7-seg digit to display
": i++ i @ 1 + 3 AND DUP i ! ;\n"                      // ( -- ) increment i and keep between 0~3
": dsp i++ DUP 7s 7d ;\n"                              // ( -- ) display 
"5 0 TMI dsp\n"                                        // timer interrupt to update display every 5ms
"VAR cnt 0 cnt !\n"                                    // cnt is a counter we want it value on display
": c++ cnt @ 1 + DUP cnt ! ?v ;\n"                     // ( -- ) increment the counter and cache patterns
"1000 1 TMI c++\n";                                    // timer interrupt to update c every second

void setup() {
    Serial.begin(115200);            ///< init Serial Monitor
    n4_setup(code);                  /// * setup nanoFORTH with preload Forth code
}

void loop() {
    n4_run();                        ///< execute VM of our NanoForth instance
}

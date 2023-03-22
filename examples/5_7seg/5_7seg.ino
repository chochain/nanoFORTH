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
": ini 12 FOR 1 I 1 + PIN NXT ; ini FGT ini\n"
"VAR x 8 ALO\n"
"$F360 x ! $B5F4 x 2 + ! $66D6 x 4 + !\n"
"$D770 x 6 + ! $F776 x 8 + !\n"
"VAR d 2 ALO $3834 d ! $2C1C d 2 + !\n"
": 7d d + C@ DUP $10C OUT $230 OUT ;\n"
"VAR vx 2 ALO\n"
": vx! 1 - vx + C! ;\n"
": ?v 4 FOR DUP 10 MOD x + C@ I vx! 10 / NXT DRP ;\n"
": 7s vx + C@ DUP $1F0 OUT $20F OUT ;\n"
"VAR v 0 v !\n"
": v++ v @ 1 + DUP v ! ?v ;\n"
"VAR i\n"
": i++ i @ 1 + 3 AND DUP i ! ;\n"
": dsp i++ DUP 7s 7d ;\n"
"1000 0 TMI v++\n"
"5 1 TMI dsp\n";

void setup() {
    Serial.begin(115200);            ///< init Serial Monitor
    n4_setup(code);                  /// * setup nanoFORTH with preload Forth code
}

void loop() {
    n4_run();                        ///< execute VM of our NanoForth instance
}

/**
 *  @file
 *  @brief nanoFORTH example - C API 
 *
 *  This Sketch add user function
 *  + toggles LEDs between pin 5 and 6
 *
 *  How To:
 *  Open Serial Monitor (or your favorate terminal emulator) as the console input to nanoFORTH
 *  + baud rate set to 115200
 *  + line ending set to Both NL & CR (if using emulator, set Add CR on, ECHO on)
 *  + type> WRD     into Serial Monitor input, you should see *cop* *blu* *red* predefined words
 *  + type> 10 cop  into Serial input, see our blinker
 */
#include <nanoFORTH.h>

const char code[] PROGMEM = "123 456 0 api\n";

void test1() {
    int a = n4_pop();
    int b = n4_pop();
    n4_push(a + b);
}

void setup() {
    Serial.begin(115200);            ///< init Serial Monitor
    
    n4_api(0, test1);
    n4_setup(code);                  /// * setup nanoFORTH with preload Forth code
}

void loop() {
    n4_run();                        ///< execute VM of our NanoForth instance
}

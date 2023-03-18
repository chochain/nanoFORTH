/**
 *  @file
 *  @brief nanoFORTH example - LED blinker
 *
 *  Assuming you have the borad hooked up with 2 LEDs on PIN 5 and PIN 6 
 *  + make sure the right resisters are in place
 *  + google Arduino+LED+project
 *
 *  This Sketch add a second user tasks on top of our blinker
 *  + the new user task toggles LEDs between pin 5 and 6
 *
 *  How To:
 *  Open Serial Monitor (or your favorate terminal emulator) as the console input to nanoFORTH
 *  + baud rate set to 115200
 *  + line ending set to Both NL & CR (if using emulator, set Add CR on, ECHO on)
 *  + type> WRD     into Serial Monitor input, you should see *cop* *blu* *red* predefined words
 *  + type> 10 cop  into Serial input, see our blinker
 */
#include <nanoFORTH.h>

const char code[] PROGMEM =          ///< define preload Forth code here
"1 5 PIN 1 6 PIN\n"                  // * set pin 5,6 for OUTPUT
": red 5 IN 1 XOR 5 OUT ;\n"         // * define word red to toggle red LED (on pin 5)
": blu 6 IN 1 XOR 6 OUT ;\n"         // * define word blu to toogle blue LED (on pin 6)
"red\n"                              // * turn red LED on
": cop FOR red blu 500 DLY NXT ;\n";

void setup() {
    Serial.begin(115200);            ///< init Serial Monitor
    n4_setup(code);                  /// * setup nanoFORTH with preload Forth code
}

void loop() {
    n4_run();                        ///< execute VM of our NanoForth instance
}

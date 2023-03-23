/**
 *  @file
 *  @brief nanoFORTH example - Blink pin 13 in parallel
 *
 *  Multi-tasking support
 *  1. initialize nanoFORTH with preload code 
 *     + define a word that blinks built-in pin 13
 *     + install the word as a timer ISR (Interrupt Service Routine)
 *     + activate timer interrupt
 *  2. nanoFORTH itself runs in parallel in Serial Monitor
 *
 *  How To:
 *  Open Serial Monitor (or your favorate terminal emulator) as the console input to nanoFORTH
 *  + baud rate set to 115200
 *  + line ending set to Both NL & CR (if using emulator, set Add CR on, ECHO on)
 *
 *  Once compiled/uploaded, you should see
 *  + some nanoFORTH init system info and the ok prompt
 *  + note that the built-in LED is blinking at 1Hz
 *  + in Serial Monitor input, type WRD and hit <return>.
 *  + nanoFORTH is multi-tasking now<return>
 */
#include <nanoFORTH.h>

const char code[] PROGMEM =        ///< define preload Forth code here
": xx 13 IN 1 XOR 13 OUT ;\n"      // * define word xx to toggle built-in LED (pin 13)
"500 0 TMI xx\n"                   // * attach xx as timer ISR in slot 0, tick every 500ms
"1 TME\n";                         // * turn on timer interrupt

void setup() {
    Serial.begin(115200);          ///< init Serial stream
    pinMode(LED_BUILTIN, OUTPUT);
    n4_setup(code);
}

void loop() {
    n4_run();                     ///< execute NanoForth instance
}

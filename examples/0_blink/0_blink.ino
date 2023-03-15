/**
 *  @file examples/0_blink/0_blink.ino
 *  @brief nanoFORTH example - Blink pin 13
 *
 *  Our first Sketch - demostrates nanoFORTH support multi-tasking
 *  1. a user task that blinks built-in pin 13
 *  2. nanoFORTH itself runs in parallel in Serial Monitor
 *
 *  How To:
 *  Open Serial Monitor (or your favorate terminal emulator) as the console input to nanoFORTH
 *  + baud rate set to 115200
 *  + line ending set to Both NL & CR (if using emulator, set Add CR on, ECHO on)
 *
 *  Once compiled/uploaded, you should see
 *  + some nanoFORTH init system info and the ok prompt
 *  + in Serial Monitor input, type WRD and hit <return>
 *  + enter the following 
 *    > : xx 13 in 1 xor 13 out ; \ toggle built-in LED
 *    > 50 xx tmi                 \ tick xx every 500ms = 50x10ms
 *    > 1 tme                     \ enable timer interrupt
 */
#include <nanoforth.h>

void setup() {
    Serial.begin(115200);         ///< init Serial IO, make sure it is set to 'Both BL & CR' to capture input

    pinMode(LED_BUILTIN, OUTPUT);
    
    n4_setup();
}

void loop() {
    n4_run();                     ///< execute NanoForth instance
}



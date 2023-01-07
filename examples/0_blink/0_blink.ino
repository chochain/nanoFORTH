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
 */
#include <nanoforth.h>

NanoForth my_n4;                  ///< our NanoForth instance
void setup() {
    Serial.begin(115200);         ///< init Serial IO, make sure it is set to 'Both BL & CR' to capture input

    if (my_n4.begin()) {          ///< initialize NanoForth and default Serial Monitor as our output
        Serial.print(F("ERROR: memory allocation failed!"));
    }
    my_n4.add(lets_blink);        ///< add the blink task to NanoForth task manager

    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    my_n4.exec();                 ///< execute VM of our NanoForth instance
}
///
/// User defined task - turn built-in LED on/off every 500ms (pin 13)
///
N4_TASK(lets_blink) {             ///< create a blinking task (i.e. built-in LED on pin 13)
    digitalWrite(LED_BUILTIN, HIGH);
    N4_DELAY(500);
    digitalWrite(LED_BUILTIN, LOW);
    N4_DELAY(500);
} N4_END;



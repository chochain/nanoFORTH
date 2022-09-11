/**
 *  @file examples/0_blink.ino
 *  @brief nanoFORTH example - Blink pin 13
 *
 *  Our first Sketch - demostrates nanoFORTH support multi-tasking
 *  + a user task that blinks built-in pin 13
 *  + nanoFORTH itself runs in parallel
 *
 *  open Serial Monitor (or your favorate terminal emulator) as the console input to nanoFORTH
 *  + baud rate set to 115200
 *  + line ending set to Both NL & CR (if using emulator, set Add CR on, ECHO on)
 *
 *  Once compiled/uploaded, you should see
 *  + some nanoFORTH init system info
 *  + ok prompt
 *  + try type WRD and hit return on the input above
 */
#include <nanoforth.h>

N4_TASK(blink)                    ///< create blinking task (i.e. built-in LED on pin 13)
{
    digitalWrite(LED_BUILTIN, HIGH);
    N4_DELAY(500);
    digitalWrite(LED_BUILTIN, LOW);
    N4_DELAY(500);
}
N4_END;

NanoForth n4;                     ///< create NanoForth instance
void setup()
{
    Serial.begin(115200);         ///< init Serial IO, make sure it is set to 'Both BL & CR' to capture input

    if (n4.begin()) {             /// default: (Serial,0x480,0x80), try reducing if memory is constrained
        Serial.print(F("ERROR: memory allocation failed!"));
    }
    n4.add(blink);                ///< add blink task to NanoForth task manager

    pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
    n4.exec();                   // execute one nanoForth VM cycle
}

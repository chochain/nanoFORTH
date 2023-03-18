/**
 * @file examples/2_bluetooth/2_bluetooth.ino
 * @brief nanoFORTH - wireless with Bluetooth HC-05
 *
 * Assume you have a HC-05 hooked-up on Pin 8 and 9
 * + google Arduino+HC-05 if not yet
 * + this example works with Android device only
 * + iPhone needs HM-10 instead because it's Bluetooth 4.0 or BLE, but same concept applies
 *
 * This Sketch demonstrates how nanoFORTH can be controlled over Bluetooth wirelessly
 * From your laptop, PC, or Android, connect to HC-05 with a terminal app.
 *   + minicom, emacs(serial-term) on Linux, Macs, or TeraTerm on PC
 *   + Serial Bluetooth Terminal on Andriod
 *   + at 9600-N-8-1 (default rate of HC-05)
 *   + set terminal to line-mode (instead of raw mode)
 *
 * After compile/upload you should see on the terminal program
 *   + nanoFORTH init system info and ok prompt
 *   + in your terminal app, try type WRD and hit <return>
 */
#include <AltSoftSerial.h>
#include <nanoforth.h>

AltSoftSerial bt;                   ///< default: RX on pin 8, TX on pin 9
NanoForth     n4;                   ///< create NanoForth instance

void setup() {
    while (!Serial);
    Serial.begin(115200);           ///< init Serial IO, make sure it is set to 'Both BL & CR' to capture input

    bt.begin(9600);                 ///< setup Bluetooth serial device at 9600 baud
    delay(1000);                    ///< wait for a while, for it to stable

    pinMode(LED_BUILTIN, OUTPUT);
    
    n4.add_task(blink13);           ///< add the blink task to NanoForth task manager
    if (n4.begin(bt)) {             ///< setup Bluetooth as nanoFORTH console
        Serial.print(F("ERROR: memory allocation failed!"));
    }
}

void loop() {
    n4.exec();                      // execute command line from BT serial console
}
///
/// User defined task - blinks built-in LED
///
N4_TASK(blink13) {                  ///< create blinking task (i.e. built-in LED on pin 13)
    digitalWrite(LED_BUILTIN, HIGH);
    N4_DELAY(500);
    digitalWrite(LED_BUILTIN, LOW);
    N4_DELAY(500);
} N4_END;



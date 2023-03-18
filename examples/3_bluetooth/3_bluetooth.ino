/**
 * @file
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

void setup() {
    Serial.begin(115200);           ///< init Serial IO
    while (!Serial);

    bt.begin(9600);                 ///< setup Bluetooth serial device at 9600 baud
    delay(1000);                    ///< wait for a while, for it to stable

    n4_setup("", 1, bt);            ///< add the blink task to NanoForth task manager
}

void loop() {
    n4_run();                       // execute command line from BT serial console
}


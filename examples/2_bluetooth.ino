/**
 * @file examples/2_bluetooth.ino
 * @brief nanoFORTH - with Bluetooth HC-05
 *
 * demonstrates how nanoFORTH can be controlled over Bluetooth
 * + HC-05 uses pin 8 for RX, pin 9 for TX
 * + From your laptop, PC, or Android, connect to HC-05 with a terminal program
 *   ** minicom, emacs(serial-term) on Linux, Macs, 
 *   ** TeraTerm on PC, or
 *   ** Serial Bluetooth Terminal on Andriod
 *   at 9600-N-8-1, line-mode
 */
#include <AltSoftSerial.h>
#include "nanoforth.h"

AltSoftSerial bt;                   ///< default: RX on pin 8, TX on pin 9
NanoForth     n4;                   ///< create NanoForth instance

N4_TASK(blink)                      ///< create blinking task (i.e. built-in LED on pin 13)
{
    digitalWrite(LED_BUILTIN, HIGH);
    N4_DELAY(500);
    digitalWrite(LED_BUILTIN, LOW);
    N4_DELAY(500);
}
N4_END;

void setup()
{
    while (!Serial);
    Serial.begin(115200);           ///< init Serial IO, make sure it is set to 'Both BL & CR' to capture input
    
    bt.begin(9600);                 ///< setup Bluetooth as serial device
    delay(1000);
    
    if (n4.begin(bt)) {             ///< setup Bluetooth as nanoFORTH console
        Serial.print(F("ERROR: memory allocation failed!"));
    }
    n4.add(blink);                  ///< add blink task to NanoForth task manager
    
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
    n4.exec();                      // execute one nanoForth command line from serial console
}



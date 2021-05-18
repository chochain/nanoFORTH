//
// nanoFORTH - Forth for Arduino Nano (and UNO)
//
#include <SoftwareSerial.h>
#include "src/nanoforth.h"

N4_TASK(blink)                    ///< create blinking task (i.e. built-in LED on pin 13)
{
    digitalWrite(LED_BUILTIN, HIGH);
    N4_DELAY(500);
    digitalWrite(LED_BUILTIN, LOW);
    N4_DELAY(500);
}
N4_END;

SoftwareSerial hm11(7, 8);
NanoForth n4;                     ///< create NanoForth instance

void setup()
{
    Serial.begin(115200);         ///< init Serial IO, make sure it is set to 'Both BL & CR' to capture input
    hm11.begin(57600);
    
    if (n4.begin()) {             /// default: (Serial,0x480,0x80), try reducing if memory is constrained
        Serial.print(F("ERROR: memory allocation failed!"));
    }
    n4.add(blink);                ///< add blink task to NanoForth task manager

    pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
    n4.step();                   // execute one nanoForth VM cycle
}



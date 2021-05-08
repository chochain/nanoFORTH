//
// nanoFORTH - Forth for Arduino Nano (and UNO)
//
#include "nanoforth.h"

NanoForth *n4;                 // create a nanoForth instance

void setup()
{
    Serial.begin(115200);
    n4 = new NanoForth();
}

void loop()
{
    n4->run();                 // execute one vm cycle
}

void n4_loop()
{
    digitalWrite(LED_BUILTIN, HIGH);
    //n4_delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    //n4_delay(500);
}


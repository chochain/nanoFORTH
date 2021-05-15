//
// nanoFORTH - Forth for Arduino Nano (and UNO)
//
#include "nanoforth.h"

N4_FUNC(blink)                 // 
{
    N4_BEGIN();
    
    digitalWrite(LED_BUILTIN, HIGH);
    N4_DELAY(500);
    digitalWrite(LED_BUILTIN, LOW);
    N4_DELAY(500);

    N4_END();
}

N4_FUNC(led_toggle)
{
    N4_BEGIN();

    digitalWrite(5, HIGH);
    digitalWrite(6, LOW);
    N4_DELAY(250);
    
    digitalWrite(5, LOW);
    digitalWrite(6, HIGH);
    N4_DELAY(250);

    N4_END();
}

void setup()
{
    Serial.begin(115200);
    
    if (N4::begin()) {
        Serial.print(F("ERROR: memory allocation failed!"));
    }
    N4::add(blink);            // add blink
    //N4::add(led_toggle);
}

void loop()
{
    N4::step();                // execute one vm cycle
}



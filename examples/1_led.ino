/**
 *  @file examples/1_led.ino
 *  @brief nanoFORTH example - LED blinker
 *
 *  Assuming you have the borad hooked up with 2 LEDs on PIN 5 and PIN 6 
 *  + make sure the right resisters are in place
 *  + google Arduino+LED+project
 *
 *  This Sketch add a second user tasks on top of our blinker
 *  + the new user task toggles between pin 5 and 6
 *
 */
#include "nanoforth.h"

N4_TASK(blink)                    ///< create blinking task (i.e. built-in LED on pin 13)
{
    digitalWrite(LED_BUILTIN, HIGH);
    N4_DELAY(500);
    digitalWrite(LED_BUILTIN, LOW);
    N4_DELAY(500);
}
N4_END;

N4_TASK(led_toggle)               ///< create a LED toggle task
{
    digitalWrite(5, HIGH);
    digitalWrite(6, LOW);
    N4_DELAY(250);
    
    digitalWrite(5, LOW);
    digitalWrite(6, HIGH);
    N4_DELAY(250);
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
    n4.add(led_toggle);           ///< add led_toggle task

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
}

void loop()
{
    n4.exec();                   // execute one nanoForth VM cycle
}



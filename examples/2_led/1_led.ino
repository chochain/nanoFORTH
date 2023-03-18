/**
 *  @file examples/1_led/1_led.ino
 *  @brief nanoFORTH example - LED blinker
 *
 *  Assuming you have the borad hooked up with 2 LEDs on PIN 5 and PIN 6 
 *  + make sure the right resisters are in place
 *  + google Arduino+LED+project
 *
 *  This Sketch add a second user tasks on top of our blinker
 *  + the new user task toggles LEDs between pin 5 and 6
 *
 */
#include <nanoforth.h>

NanoForth n4;                     ///< our NanoForth instance
void setup() {
    Serial.begin(115200);         ///< init Serial IO, make sure it is set to 'Both BL & CR' to capture input

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    
    n4.add_task(blink13);         ///< add blink13 task to NanoForth task manager
    n4.add_task(led_toggle);      ///< add the second task: led_toggle

    if (n4.begin()) {             /// initialize NanoForth and Serial Monitor as output
        Serial.print(F("ERROR: memory allocation failed!"));
    }
}

void loop() {
    n4.exec();                   /// execute VM of our NanoForth instance
}
///
/// User defined task1 - blink built-in LED
///
N4_TASK(blink13) {               ///< the built-in LED blinking task
    digitalWrite(LED_BUILTIN, HIGH);
    N4_DELAY(500);
    digitalWrite(LED_BUILTIN, LOW);
    N4_DELAY(500);
} N4_END;
///
/// User defined task2 - toggle LEDs on pin 5 and 6
///
N4_TASK(led_toggle) {            ///< create a task that toggle LEDs
    digitalWrite(5, HIGH);
    digitalWrite(6, LOW);
    N4_DELAY(250);

    digitalWrite(5, LOW);
    digitalWrite(6, HIGH);
    N4_DELAY(250);
} N4_END;


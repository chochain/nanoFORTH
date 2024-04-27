/**
 *  @file
 *  @brief Temperature & Humidity Measure
 *
 *  Assuming you have a DHT11 sensor on pin 12
 *
 *  How To:
 *  Open Serial Monitor (or your favorate terminal emulator) as the console input to nanoFORTH
 *  + baud rate set to 115200
 *  + line ending set to Both NL & CR (if using emulator, set Add CR on, ECHO on)
 */
#include <nanoFORTH.h>

#include <dht.h>
#define DHT11_PIN 12
dht DHT;

const char code[] PROGMEM =
": dht ( -- t h ) 0 API ;\n"
": dsp dht .\" H=\" . .\" T=\" . CR ;\n"
"2000 0 TMI dsp\n"
".\" type 1 TME to start reading, and 0 TME to stop \"\n";

void dht() {
    int chk = DHT.read11(DHT11_PIN);
    if (chk == DHTLIB_OK) {
        n4_push(static_cast<int>(DHT.temperature));
        n4_push(static_cast<int>(DHT.humidity));
    }
    else {
        n4_push(99);
        n4_push(99);
    }
    digitalWrite(13, digitalRead(13)^1);
}

void setup() {
    Serial.begin(115200);  ///< init Serial Monitor

    n4_api(0, dht);        ///< register DHT func
    n4_setup(code);
}

void loop() {
    n4_run();              ///< execute VM of our NanoForth instance
}

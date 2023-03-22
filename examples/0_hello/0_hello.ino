/**
 *  @file
 *  @brief nanoFORTH example - plain FORTH 
 *  Our first Sketch - hello world with nanoFORTH itself runs in Serial Monitor
 *
 *  How To:
 *  Open Serial Monitor (or your favorate terminal emulator) as the console input to nanoFORTH
 *  + baud rate set to 115200
 *  + line ending set to Both NL & CR (if using emulator, set Add CR on, ECHO on)
 *
 *  Once compiled/uploaded, you should see
 *  + some nanoFORTH init system info and the ok prompt
 *  + in Serial Monitor input, type WRD and hit <return>
 */
#include <nanoFORTH.h>

void setup() {
    Serial.begin(115200);          ///< init Serial stream
    n4_setup("");                  ///< intialize nanoFORTH
}

void loop() {
    n4_run();                      ///< execute NanoFORTH
}

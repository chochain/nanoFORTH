/**
 *  @file
 *  @brief nanoFORTH example - C API 
 *
 *  This Sketch add user functions and call them from within nanoForth
 *
 *  How To:
 *  Open Serial Monitor (or your favorate terminal emulator) as the console input to nanoFORTH
 *  + baud rate set to 115200
 *  + line ending set to Both NL & CR (if using emulator, set Add CR on, ECHO on)
 *  + type> WRD                   \ note that there is no new word created in dictionary
 *  + type> 123 456 m+            \ calls my_add(123, 456) and put 579 on top of stack
 *  + type> : m- 1 API ;          \ define a word to wrap the API if you want to
 *  + type> 123 m-                \ calls my_sub(579, 123) and return 456 on stack
 */
#include <nanoFORTH.h>

void my_add() {
    int b = n4_pop();
    int a = n4_pop();
    n4_push(a + b);
}

void my_sub() {
    int b = n4_pop();
    int a = n4_pop();
    n4_push(a - b);
}

void setup() {
    Serial.begin(115200);            ///< init Serial Monitor
    
    n4_api(0, my_add);               ///< register my_add as API[0]
    n4_api(1, my_sub);               ///< register my_sub as API[1]
    n4_setup();                      /// * setup nanoFORTH with preload Forth code
}

void loop() {
    n4_run();                        ///< execute VM of our NanoForth instance
}

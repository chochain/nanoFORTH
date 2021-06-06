//
// nanoFORTH - Forth for Arduino Nano (and UNO)
//
#include <AltSoftSerial.h>
#include "src/nanoforth.h"

N4_TASK(blink)                      ///< create blinking task (i.e. built-in LED on pin 13)
{
    digitalWrite(LED_BUILTIN, HIGH);
    N4_DELAY(500);
    digitalWrite(LED_BUILTIN, LOW);
    N4_DELAY(500);
}
N4_END;

AltSoftSerial bt;                   ///< default: RX on pin 8, TX on pin 9
NanoForth     n4;                   ///< create NanoForth instance
/* 
   JDY-10 (google '4.0 module ble bluetooth serial port module')
   AT command 
   * available only when not connect to other device
   * default at 115200N81
   * command terminated with \r\n
   AT+VER      query version     => JDY-10-V2.5
   AT+MAC      query mac address
   AT+NAME     qeury device name
   AT+NAME[..] set device name
   AT+BAUD     query baud rate
   AT+BAUDn    set baud rate 0:115200, 1:57600, 2:38400, 3:19200, 4:14400, 5:9600
   AT+POWE     qeury tx power 
   AT+ADVINTn (0-6) 100ms~4800ms
*/
void device_info(Stream &bt)
{
    const char *cmd_lst[] = {
        "VER",
        "MAC",
        "NAME",
        "BAUD",
        "POWE",
        "ADVINT"
    };
    Serial.println("Device Info:");
    for (int i=0; i<sizeof(cmd_lst)/sizeof(const char*); i++) {
        const char *cmd = cmd_lst[i];
        Serial.print("  "); Serial.print(cmd); Serial.print("=>");
        delay(1000);
        bt.write("AT+"); bt.write(cmd); bt.write("\r\n");
        delay(600);
        while (bt.available()) {
            Serial.write(bt.read());
        }
    }
}

void setup()
{
    while (!Serial);
    Serial.begin(115200);           ///< init Serial IO, make sure it is set to 'Both BL & CR' to capture input
    
    bt.begin(9600);
    delay(1000);
    /*
    device_info(bt);
    for (int i=0; i<0x80; i++) {
        bt.write((U8)i|0x80);
    }
    return;
    */  
    if (n4.begin(bt)) {             /// default: (Serial,1,0x480,0x80), try reducing if memory is constrained
        Serial.print(F("ERROR: memory allocation failed!"));
    }
    n4.add(blink);                  ///< add blink task to NanoForth task manager

    pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
    /*
    if (Serial.available()) {   // send Serial console input to Bluetooth device
        Serial.read();
        bt.write("AT+BAUD5\r\n");
    }
    if (bt.available()) {
        Serial.write(bt.read());
    }
    */
    n4.exec();                  // execute one nanoForth command line from console
}



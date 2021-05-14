\page page1 What

## nanoFORTH - a simple and useful FORTH for Arduino Nano
### Assumptions
* more than 80% of Arduino makers are using UNO or Nano,
* most of the makers do not need the full blown FORTH vocabularies,
* most of them are not familer with standard FORTH words, so abbriviation for words is OK,
* the meta-compiler is unlikely needed either, i.e. not to create a new type of Forth from within nanoForth,
* only a small set of core primitive words are needed for the most of Arduino projects,
  the rational being anything that requires more complicated syntax, one might need the power of ESP.

### Requirements
* be as simple to use as any example Sketch that comes with the IDE (no bootloader burning),
* provide a REPL development/operating environment for Arduino,
* provide core Arduino functions (i.g. pinMode, digitalRead,Write, analogRead,Write, millis, delay),
* provide hardware thread(s) in addition to nanoFORTH thread so new components can be added (ig. Bluetooth),
* provide at least 1K RAM dictionary for resonablly size of work,
* provide EEPROM persisted storage for new words which can be automatically reloaded on next start-up,
* optionally show byte-code stream while assembled to help beginers understand FORTH internal,
* optionally show execution tracing to help debugging, also provision for single-stepping,
* optionally implemented as an Arduino library that developers can include easily.

### Installations
todo: wait until Arduino forum accept the package

### Examples
* turn on LED(red) on digital pin 5
> 5 1 OUT

* turn off LED(blue) on digital pin 6
> 6 0 OUT

* define a function(called 'word' in FORTH) **red** to turn red LED on, and blue LED off
> : **red** 5 1 OUT 6 0 OUT ;

* define a word **blu** to turn red LED off and turn blue LED on (sorry, nanoFORTH takes max 3 characters only)
> : **blu** 5 0 OUT 6 1 OUT ;

* execute **blu**, i.e. to turn red LED off, and blue LED on 
> **blu**

* define a word **xy** to blink red/blue every 500 ms alternatively
> : **xy** 0 FOR **red** 500 DLY **blu** 500 DLY NXT ;

* run 10 cycles of **xy**
> 10 **xy**

* too slow! nanoFORTH lets you redefine **xy** (ig. to make it faster and uneven)
> FGT **xy**<br>
> : **xy** 0 FOR I . **red** 200 DLY **blu** 400 DLY NXT ;

* try 20 cycles of **xy** this time
> 20 **xy**

* let's read analog pin 1 (photoresister value 0~1023)
> 1 AIN<br>
> 258_ok

* define (a word) **lit** to read from photoresister and determine whether its value is > 200
> : **lit** 1 AIN 200 > ;

* execute **lit**, return value 1 if bright enough, 0 if not
> **lit**<br>
> 1_ok

* define **XY** that turns on red or blue depends on light condition (nanoFORTH is case sensitive)
> : **XY** **lit** IF **red** ELS **blu** THN ;

* run **XY** to turns on red or blue (try blocking the photoregister)
> **XY**

* show all words available, including **XY**, **xy**, **lit**, **blu**, **red** that we've just created
> WRD

* now, to see what nanoFORTH did, turn the tracing flag on and try everything we just did again
> 1 TRC

***

* find out how many bytes of memory we have used
> HRE<br>
> 76_ok

* you can dump the memory to see how all these words are encoded in the dictionary
> 0 HRE DMP

* let's save what we've done so far into EEPROM, in case of power outage (or reset)
> SAV

* if really needed, you can reset nanoFORTH system pointers for a clean slate
> BYE<br>
> nanoFORTH v1.0 ok

* after restart you Arduino, words defined can be restored from EEPROM
> LD<br>
> 0 HRE DMP




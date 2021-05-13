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

* define a function(called 'word' in FORTH) **p5** to turn red LED on, and blue LED off
> : **p5** 5 1 OUT 6 0 OUT ;

* define a word **p6** to turn red LED off and turn blue LED on
> : **p6** 5 0 OUT 6 1 OUT ;

* execute **p5**, i.e. to turn red LED on, and blue LED off
> **p5**

* to read the photoresister value from analog pin 1
> 1 AIN<br>
> 258_ok

* define (a word) **lit** to read from photoresister and determine whether its value is > 200
> : **lit** 1 AIN 200 > ;

* execute **lit**, put -1 on top of stack if bright enough, 0 if not (FORTH uses -1 instead of 1 for TRUE)
> **lit**<br>
> -1_ok

* define **xx** that turns on red or blue depends on light condition
> : **xx** **lit** IF **p5** ELS **p6** THN ;

* run **xx** to turns on red or blue (try blocking the photoregister)
> **xx**

* define a word **yy** to blink red/blue every 500 ms alternatively
> : **yy** 0 FOR **p5** 500 DLY **p6** 500 DLY NXT ;

* run 100 cycles of **yy**
> 100 **yy**


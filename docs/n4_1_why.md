\page page1 Why

The emerging microcontroller communities are being built no more around specific hardware form factor, but rather around higher level languages. Without introducing an interactive shell like Javascript or microPython supported by ESP or Raspberry, the once popular Arduino platform will gradually lose out the market. However, no matter how the hardware environment evolved, on the edge of IoT universe, a mimimalist system resemble the Aruino UNO will always have its value of existance provided that some of the form-factor might one-day shrunk down to micro or even nano-scale. Being chip argnostic, the Arduino IDE does serve as an excellent learning tool for future systems to come. Factor software develpment time into the mix, an interactive/interpreted language is not only cheaper but more fun than the good old static compiled C code in many cases. Check out <a href="https://www.forth.com/resources/space-applications" target="_blank">NASA</a> and contemplate why FORTH is still running on a number of space probes today.

Following the footstep of <a href="http://middleriver.chagasi.com/electronics/tforth.html" target="_blank">Nakagawa</a> and <a href="https://circuit4us.medium.com/tiny-forth-with-arduino-hardware-support-255f408b745a" target="_blank">circuit4u@medium.com's</a> **TinyForth**, a light-weight protothreaded FORTH with 3-character keywords for Arduino, I got an idea!

## nanoFORTH - a simple and useful FORTH for Arduino Nano
### Assumptions
* more than 80% of Arduino makers are using UNO or Nano,
* most of the makers do not need the full-blown FORTH vocabularies,
* most of them are not familer with standard FORTH words, so abbriviation for words is OK,
* the meta-compiler is unlikely needed either, i.e. not to create a new type of Forth from within nanoForth,
* only a small set of, say 50+, core primitive words are needed for the most of Arduino projects,
  the rational being anything that requires more elaborated syntax, one might need the power of ESPs.

### Requirements
* be as simple to use as any example Sketch that comes with the IDE (no bootloader burning),
* provide a REPL development/operating environment for Arduino,
* provide core Arduino functions (i.g. pinMode, digitalRead,Write, analogRead,Write, millis, delay),
* provide hardware thread(s) in addition to nanoFORTH thread so new components can be added (ig. Bluetooth),
* provide an Arduino library that developers can include easily,
* provide at least 1K RAM dictionary for resonablly size of work,
* provide EEPROM persisted storage for new words which can be automatically reloaded on next start-up,
* optionally show byte-code stream while assembled to help beginers understand FORTH internal,
* optionally show execution tracing to help debugging, also provision for single-stepping.

### Use Cases - Interaction Examples
* turn on LED(red) on digital pin 5, 1 is HIGH
> 1 5 OUT ⏎

* turn off LED(blue) on digital pin 6, 0 is LOW
> 0 6 OUT ⏎

* define a function, or a 'word' in FORTH, **red** to turn red LED on, and blue LED off
> : **red** 1 5 OUT 0 6 OUT ; ⏎

* define a word **blu** to turn red LED off and turn blue LED on (sorry, nanoFORTH takes max 3 characters only)
> : **blu** 1 6 OUT 1 5 OUT ; ⏎

* execute **blu**, i.e. to turn red LED off, and blue LED on 
> **blu** ⏎

* define a word **xy** to blink red/blue every 500 ms alternatively
> : **xy** 0 FOR **red** 500 DLY **blu** 500 DLY NXT ; ⏎

* run 10 cycles of **xy**
> 10 **xy** ⏎

* too slow! nanoFORTH lets you redefine **xy** by "forget" it first
> FGT **xy** ⏎<br>
> : **xy** 0 FOR **red** 200 DLY **blu** 400 DLY I . NXT ; ⏎

* now try 20 cycles of **xy** this time
> 20 **xy** ⏎

* let's read analog pin 1 (photoresister value 0~1023)
> 1 AIN ⏎<br>
> 258_ok

* define **lit** to read from photoresister and determine whether its value is > 200
> : **lit** 1 AIN 200 > ; ⏎

* execute **lit**, return value 1 on stack (FORTH's memory) if it's bright enough, 0 if not
> **lit** ⏎<br>
> 1_ok

* define **?Z** that turns on red or blue depends on input value on stack. unlike other languages, you can create really strange function names
> : **?Z** IF **red** ELS **blu** THN ; ⏎

* run **?Z** which take input from stack, 1 turns on red or 0 turn on blue
> 1 **?Z** ⏎<br>
> 0 **?Z** ⏎

* now we can turn on red or blue LED depends on lighting condition (try blocking the photoresister), **lit** leave 1 or 0 on stack, **?Z** takes the value and turns on red or blue
> **lit** **?Z** ⏎

* if you really want to, we can even make it into an infinite loop. But why? OK, running over-night or maybe something like a web-server.
> : **xyz** BGN **lit** **?Z** RPT ; ⏎<br>
> 100 **xyz**

* show all words available, including **?Z**, **xy**, **lit**, **blu**, **red** that we've just created
> WRD ⏎

OK, if that captured the imaginations, we might have an idea of what nanoFORTH is trying to do. Remember, we do these without any compilation but, instead, "talk" directly with Arduino once nanoFORTH uploaded via the USB cable. The interactive nature changes the way we are so used to. Further more, what if we can do it via WiFi or BLE? Look Mom! I can talk to the mailbox. No cable!

<br/>
<a href="page2.html">Ready to get nanoFORTH for a trial?</a><br/>
<a href="page3.html">References and all the details...</a>

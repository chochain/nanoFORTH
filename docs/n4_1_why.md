\page page1 .1 Why

The emerging micro-controller communities are being built no more around specific hardware form factor, but rather around higher level languages. Without introducing an interactive shell like Javascript or microPython supported by ESP or Raspberry, the once popular Arduino platform will gradually lose out the market. However, no matter how the hardware environment evolved, on the edge of IoT universe, a minimalist system resembles the Aruino UNO will always have its value of existence provided that some of the form-factor might one-day shrunk down to micro or even nano-scale. Being chip agnostic, the Arduino IDE does serve as an excellent learning tool for future systems to come. Factor software development time into the mix, an interactive/interpreted language is not only cheaper but more fun than the good old static compiled C code in many cases. Check out <a href="https://www.forth.com/resources/space-applications" target="_blank">NASA</a> and contemplate why FORTH is still running on a number of space probes today.

Following the footsteps of <a href="http://middleriver.chagasi.com/electronics/tforth.html" target="_blank">Nakagawa</a> and <a href="https://circuit4us.medium.com/tiny-forth-with-arduino-hardware-support-255f408b745a" target="_blank">circuit4u@medium.com's</a> **TinyForth**, a light-weight protothreaded FORTH with 3-character keywords for Arduino, I got an idea!

## nanoFORTH - a simple and useful FORTH for Arduino Nano
### Assumptions
* more than 80% of Arduino makers are using UNO or Nano,
* most of the makers do not need the full-blown FORTH vocabularies,
* most of them are not familiar with standard FORTH words, so abbreviation for words is OK,
* the meta-compiler is unlikely needed either, i.e. not to create a new type of Forth from within nanoForth,
* only a small set of, say 50+, core primitive words are needed for the most of Arduino projects,
  the rationale being anything that requires more elaborated syntax, one might need the power of ESPs.

### Requirements
* be as simple to use as any example Sketch that comes with the IDE (no bootloader burning),
* provide a REPL development/operating environment for Arduino,
* provide core Arduino functions (i.g. pinMode, digitalRead/Write, analogRead/Write, millis, delay),
* provide hardware thread(s) in addition to the nanoFORTH thread so that new components can be added (ig. Bluetooth),
* provide an Arduino library that developers can include easily,
* provide at least 1K RAM dictionary for reasonably size of work,
* utilize EEPROM as the persistant storage for user defined words that can be reloaded after restart,
* show assembly trace (i.e. byte-code stream) to help beginners to understand FORTH internal,
* show execution trace to enable debugging, also provision for single-stepping.
* capable of autorun after reboot (from saved EEPROM image).

### Use Cases - Interaction Examples
* turn on LED(red) on digital pin 5, or imagine you have a board hooked up like this, (1 is HIGH)
> 1 5 OUT ⏎
> ||
> |:--|
> |@image html images/nanoforth_led_red.jpg width=200px|
<br/>

* turn off LED(blue) on digital pin 6, (0 is LOW)
> 0 6 OUT ⏎

* define a function, or a 'word' in FORTH, **red** to turn red LED on, and blue LED off
> : **red** 1 5 OUT 0 6 OUT ; ⏎
>> \> the symbol : starts the definition, and ; ends the function (or word) definition

* define another word **blu** to turn red LED off and turn blue LED on (sorry, no blue, nanoFORTH takes max 3 characters only)
> : **blu** 0 5 OUT 1 6 OUT ; ⏎

* execute **blu**, i.e. to turn red LED off, and blue LED on 
> **blu** ⏎
>> \> a function is defines in the 'Compile Mode', and executed in 'Interpreter Mode'. The differece is at the leading ':' (colon) sign.

* define a word **xy** to blink red/blue every 500 ms alternatively
> : **xy** FOR **red** 500 DLY **blu** 500 DLY NXT ; ⏎

* run 10 cycles of **xy**
> 10 **xy** ⏎
> ||
> |:--|
> |@htmlonly <iframe width="400" height="320" src="https://www.youtube.com/embed/trmDNh41-pQ?version=3&playlist=trmDNh41-pQ&loop=1&controls=0" title="" frameborder="0" allow="autoplay; picture-in-picture" allowfullscreen></iframe> @endhtmlonly|
>> \> so, 10 FOR ... NXT is to loop 10 times, (counting down from 10, 9, 8, ..., 2, 1)

* if that's a bit too slow! nanoFORTH allows you redefine **xy** by "forget" it first
> FGT **xy** ⏎<br/>
>> \> that erased **xy** from memory, we can redefine it now<br/>
>> \> actually, multiple definition of the same function is allowed, the latest one takes precedence.<br/>
>> \> also, FGT a word that is an interrupt service (see page3) might cause undefined behaviour
>
> : **xy** FOR **red** 200 DLY **blu** 300 DLY **I .** NXT ; ⏎<br/>

* now try 20 cycles of **xy** this time
> 20 **xy** ⏎
> ⇨ 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 ok
>> \> so, you've probably noticed that **I** is the loop counter and . (dot) prints it<br/>

* let's try analog, say read a value from analog pin 1 (photoresister value 0~1023), assuming you have one installed
> 1 AIN ⏎<br>
> ⇨ 258_ok
>> \> 258 is the value nanoFORTH read from photoresister, then place it on top of data stack

* we don't need the value 258, let's drop it from data stack to keep it clean
> DRP ⏎<br>
> ⇨ ok
>> \> 258 is gone now

* define **lit** to read from photoresister and determine whether its value is > 200
> : **lit** 1 AIN 200 > ; ⏎

* execute **lit**, it puts value 1 on data stack (FORTH's memory) if your room is bright enough, a value 0 otherwise
> **lit** ⏎<br>
> ⇨ 1_ok

* define **?Z** that turns on red or blue depends on value on top of data stack. 
> : **?Z** IF **red** ELS **blu** THN ; ⏎
>> \> **?Z** is our newly defined function. Unlike most of the other languages, you can create some really strange function names in FORTH.

* run **?Z** which read from top of data stack, if it's 1 then turns on red LED or 0 turns on blue
> 1 **?Z** ⏎<br>
> 0 **?Z** ⏎

* now we may turn on red or blue LED depending on lighting condition (try blocking the photoresister), **lit** leaves 1 or 0 on data stack, **?Z** takes the value and turns on the red or blue LED
> **lit** **?Z** ⏎

* define a word **xyz** to check photoresister in a loop every 1 second, turn the blue or red LED on depending on the photoresister value read
> : **xyz** FOR **lit** **?Z** 1000 DLY NXT ; ⏎<br>
> 60 **xyz** ⏎
>> \> This runs **xyz** for a minute. Try blocking the photoresister to see the LED toggles.<br/>
>> \> Can this become a trigger i.e. mouse trap or something useful?<br/>
>> \> Make it run in an infinite loop like a web-server? Sure, but we will leave that detail to future chapter.<br/>
>> \> Have you noticed the Pin 13 green LED is blinking at its own pace?

* show all nanoFORTH words available, including **xyz**, **?Z**, **xy**, **lit**, **blu**, **red** that we've just created
> WRD ⏎
> ||
> |:--|
> |@image html images/nanoforth_wrd_list.png width=800px|
>> \> Behold! This is nanoFORTH in its entirety. It's a short list of 'words' which should be rather easy to master.
<br/>

OK, if that have captured the imaginations, we might have an idea of what nanoFORTH is trying to do. Remember that we do these without any compilation, instead, "talk" directly with Arduino once nanoFORTH uploaded via the USB cable. The interactive nature changes the way we are very used to on this platform. Imagine, what if we can do it via WiFi or BLE using our Nano? Look Mom! I can talk to the mailbox. No cable!

<br/>
<a href="page2.html">2. How - Ready to get nanoFORTH for a trial?</a><br/>
<a href="page3.html">3. What - References and all the details...</a>

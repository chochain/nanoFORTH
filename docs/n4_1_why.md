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
* provide an Arduino library that developers can include easily,
* provide at least 1K RAM dictionary for reasonably size of work,
* utilize EEPROM as the persistant storage for user defined words that can be reloaded after restart,
* provide C API so that user defined functions/components can be integrated (ig. Servo, Bluetooth, ...),
* provide timer interrupt handler to support multi-tasking,
* privide pin change interrupt handler to support hardware trigger,
* show assembly trace (i.e. byte-code stream) to help beginners to understand FORTH internal,
* show execution trace to enable debugging, also provision for single-stepping.
* capable of autorun after reboot (from saved EEPROM image).

### Use Cases - Interaction Examples
* Turn on LED(red) on digital pin 5, imagine you have a board hooked up, or try <a href="https://wokwi.com/projects/359920992049600513" target="_blank">check this Wokwi project</a>
> 1 5 OUT ⏎
> ||
> |:--|
> |@image html images/nanoforth_led_red.jpg width=200px|
<br/>

* Turn off LED(blue) on digital pin 6, (0 is LOW).
> 0 6 OUT ⏎

* Tefine a function, or a 'word' in FORTH, **red** to turn red LED on, and blue LED off.
> : **red** 1 5 OUT 0 6 OUT ; ⏎
>> \> the symbol : starts the definition, and ; ends the function (or word) definition

* Define another word **blu** to turn red LED off and turn blue LED on (sorry, no blue, nanoFORTH takes max 3 characters only).
> : **blu** 0 5 OUT 1 6 OUT ; ⏎

* Execute **blu**, i.e. to turn red LED off, and blue LED on.
> **blu** ⏎
>> \> a function is defines in the 'Compile Mode', and executed in 'Interpreter Mode'. The differece is at the leading ':' (colon) sign.

* Define a word **xy** to blink red/blue every 500 ms alternatively.
> : **xy** FOR **red** 500 DLY **blu** 500 DLY NXT ; ⏎

* Run 10 cycles of **xy**.
> 10 **xy** ⏎
> ||
> |:--|
> |@htmlonly <iframe width="400" height="320" src="https://www.youtube.com/embed/trmDNh41-pQ?version=3&playlist=trmDNh41-pQ&loop=1&controls=0" title="" frameborder="0" allow="autoplay; picture-in-picture" allowfullscreen></iframe> @endhtmlonly|
>> \> so, 10 FOR ... NXT is to loop 10 times, (counting down from 10, 9, 8, ..., 2, 1)

* If that's a bit too slow! nanoFORTH allows you redefine **xy** by "forget" it first.
> FGT **xy** ⏎<br/>
>> \> that erased **xy** from memory, we can redefine it now<br/>
>> \> actually, multiple definition of the same function is allowed, the latest one takes precedence.<br/>
>> \> also, FGT a word that is an interrupt service (see page3) might cause undefined behaviour
>
> : **xy** FOR **red** 200 DLY **blu** 300 DLY **I .** NXT ; ⏎<br/>

* Now, try 20 cycles of **xy** this time.
> 20 **xy** ⏎
> ⇨ 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 ok
>> \> so, you've probably noticed that **I** is the loop counter and . (dot) prints it<br/>

* Let's try analog, say read a value from analog pin 1, assuming you have one installed, or again try <a href="https://wokwi.com/projects/359920992049600513" target="_blank">this Wokwi project</a>
> 1 AIN ⏎<br>
> ⇨ 258_ok
>> \> 258 is the value nanoFORTH read from photoresister, then place it on top of data stack
>> \> a photoresister or potentiometer returns value between 0 and 1023

* If we don't need the value 258, we can drop it from data stack to keep it clean
> DRP ⏎<br>
> ⇨ ok
>> \> 258 is gone now

* Define **lit** to read from photoresister (or a potentiometer) and determine whether its value is > 200.
> : **lit** 1 AIN 200 > ; ⏎

* Execute **lit**, it puts value 1 on data stack (FORTH's memory) if your room is bright enough, a value 0 otherwise.
> **lit** ⏎<br>
> ⇨ 1_ok

* Define **?z** that turns on red or blue depends on value on top of data stack. 
> : **?z** IF **red** ELS **blu** THN ; ⏎
>> \> **?z** is our newly defined function. Unlike most of the other languages, you can create some really strange function names in FORTH.

* Run **?z** which read from top of data stack, if it's 1 then turns on red LED or 0 turns on blue. Try these
> 1 **?z** ⏎<br>
> 0 **?z** ⏎

* We now can turn on red or blue LED depend on lighting condition (try blocking the photoresister), **lit** leaves 1 or 0 on data stack, **?z** takes the value and turns on the red or blue LED.
> **lit** **?z** ⏎

* Define a word **xyz** to keep checking photoresister, turn the blue or red LED on depending on the photoresister value read until button hooked at pin 7 is pushed.
> : **xyz** BGN **lit** **?z** 7 IN UTL ; ⏎<br>
> **xyz** ⏎
>> \> Try blocking the photoresister to see the LED toggles.<br/>
>> \> Can this become a trigger i.e. mouse trap or something useful? Why not!<br/>

* Let's list all nanoFORTH words available in its dictionary.
> WRD ⏎
> ||
> |:--|
> |@image html images/nanoforth_wrd_list.png width=800px|
>> \> See the latest, they include **xyz**, **?z**, **xy**, **lit**, **blu**, **red** that we've just created.

Behold! This is nanoFORTH in its entirety. It's a short list of 'words' which should be rather easy to master. Note that the steps illustrated above has been the way Forth programmers building their applications. One small word at a time. Debug each well interactively then combine them into a "bigger" word. If a bug found, FGT the word, redefine it. Next word!

<br/>

OK! If the process shown above has captured the essense, we should have an idea of what nanoFORTH is trying to do. Let's just stop and contemplate for a while. We did all of the above without any recompilation. Instead, we "talked" directly with the nanoFORTH uploaded only once via the USB cable. Should you code these in C, how do you go about doing it?

The interactive nature is different from the way we are so used to on Arduino platform. Just consider how many times you have to compile your C code to go through the functions shown above. So, move forward, let's envision how we can control robots or what we can do using Bluetooth with our Nano...

<br/>
<a href="page2.html">2. How - Ready to get nanoFORTH for a trial?</a><br/>
<a href="page3.html">3. What - References and all the details...</a>

The emerging micro-controller communities are no more focusing on specific hardware or form factor but around tool-chain provided typically with higher-level languages. The once popular Arduino platform gradually lose out the market share to ESP, Raspberry, and whatever new kids on the block. Being chip agnostic, the Arduino IDE does serve as an excellent learning tool for future systems to come. No matter how the hardware environment evolved, especially on the edge of IoT universe, I felt a minimalist system similar to the Arduino UNO/Nano will always have its value especially in learning even when the size of chips shrunk down to micro or even nano-scale one-day.

On the language end, check out <a href="https://www.forth.com/resources/space-applications" target="_blank">NASA</a> and contemplate why FORTH is still running on a number of space probes today. It's known that projects developed in interactive/incremental style can be more productive compared to typical static tool-chain such as the C/C++ provided by Arduino.

I touched FORTH briefly years ago back in school days. After seeing <a href="http://middleriver.chagasi.com/electronics/tforth.html" target="_blank">Nakagawa's TinyForth</a>, I got this idea!

## nanoFORTH - FORTH for Arduino Nano
### Observations

    + many Arduino makers are still using UNO or Nano,
    + most do not need the full-blown FORTH vocabularies,
      50+ words or so, anything more might need ESP or RPi.
    + most are not familiar with standard FORTH words, abbreviation would be OK,
    + meta-compiler not needed, i.e. no trying to build a new Forth

### Features

    + no bootloader burning needed, so no extra cost or bricking,
    + a downloadable library from Arduino IDE,
    + easy to follow .ino Sketch examples, (bluetooth, 7-seg, robot)
    + 1K RAM dictionary, and EEPROM to persist user defined words,
    + call Arduino functions, (i.g. pinMode, digitalRead/Write, analogRead/Write, millis, delay),
    + C API, for user defined functions (ig. Bluetooth, Servo, ...),
    + timer ISR, support multi-tasking,
    + pin change ISR, support hardware trigger,
    + memory dump and execution trace, to understand FORTH internal,
    + autorun after reboot, can become a turnkey system

### Use Cases - Interaction Examples
* Turn on LED(red) on digital pin 5, imagine you have a board hooked up, or [check this Wokwi project](https://wokwi.com/projects/359920992049600513)
> 1 5 OUT ⏎
> |Red LED on|
> |:--|
> |<img src="https://chochain.github.io/nanoFORTH/images/nanoforth_led_red.jpg" width=200px />|
<br/>

* Turn off LED(blue) on digital pin 6, (0 is LOW).
> 0 6 OUT ⏎

* Define a function, or a 'word' in FORTH, **red** to turn red LED on, and blue LED off.
> : **red** 1 5 OUT 0 6 OUT ; ⏎
>> \> the symbol : starts the definition, and ; ends the function (or word) definition

* Define another word **blu** to turn red LED off and turn blue LED on (sorry, no blue, nanoFORTH takes max 3 characters only).
> : **blu** 0 5 OUT 1 6 OUT ; ⏎

* Execute **blu**, i.e. to turn red LED off, and blue LED on.
> **blu** ⏎
>> \> a function is defines in the 'Compile Mode', and executed in 'Interpreter Mode'. The difference is at the leading ':' (colon) sign.

* Define a word **xy** to blink red/blue every 500 ms alternatively.
> : **xy** FOR **red** 500 DLY **blu** 500 DLY NXT ; ⏎

* Run 10 cycles of **xy**.
> 10 **xy** ⏎
> |Click for Video|
> |:--|
> |[![Video](http://i3.ytimg.com/vi/trmDNh41-pQ/hqdefault.jpg)](https://www.youtube.com/embed/trmDNh41-pQ?version=3&playlist=trmDNh41-pQ&loop=1&controls=0)|
>> \> so, 10 FOR ... NXT is to loop 10 times, (counting down from 10, 9, 8, ..., 2, 1)

* If that flashes too slow for you, nanoFORTH allows redefining **xy** by "forget" it first.
> FGT **xy** ⏎<br/>
>> \> that erased **xy** from memory, we can redefine it now<br/>
>> \> actually, multiple definition of the same function is allowed, the latest one takes precedence.<br/>
>> \> also, FGT a word that is an interrupt service (see [page3](https://chochain.github.io/nanoFORTH/html/page3.html)) might cause undefined behavior
>
> : **xy** FOR **red** 200 DLY **blu** 300 DLY **I .** NXT ; ⏎<br/>

* Now, try 20 cycles of **xy** this time.
> 20 **xy** ⏎
> ⇨ 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 ok
>> \> so, you've probably noticed that **I** is the loop counter and . (dot) prints it<br/>

* Let's try analog, say read a value from analog pin 1, assuming you have one installed, or [try this Wokwi project](https://wokwi.com/projects/359920992049600513) again
> 1 AIN ⏎<br>
> ⇨ 258_ok
>> \> 258 is the value nanoFORTH read from photo-resister, then place it on top of data stack
>> \> a photo-resister or potentiometer returns value between 0 and 1023

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
> |nanoFORTH words|
> |:--|
> |<img src="https://chochain.github.io/nanoFORTH/images/nanoforth_wrd_list.png" width=800px />|
>> \> See the latest, they include **xyz**, **?z**, **xy**, **lit**, **blu**, **red** that we've just created.

Behold! This is nanoFORTH in its entirety. It's a short list of 'words' which should be rather easy to master. Note that the steps illustrated above has been the way Forth programmers building their applications. One small word at a time. Debug each well interactively then combine them into a "bigger" word. If a bug found, FGT the word, redefine it. Next word!

<br/>

OK! If the process shown above has captured the essence, we should have an idea of what nanoFORTH is trying to do. Let's just stop and contemplate for a while. We did all of the above without any recompilation. Instead, we "talked" directly with the nanoFORTH uploaded only once via the USB cable. Should you code these in C, how do you go about doing it?

The interactive nature is different from the way we are so used to on Arduino platform. Just consider how many times you have to compile your C code to go through the functions shown above. So, move forward, let's envision how we can control robots or what we can do using Bluetooth with our Nano...

<br/>
Ready to <a href="https://chochain.github.io/nanoFORTH/html/page2.html" _target="_blank">give nanoFORTH a trial?</a> or view <a href="https://chochain.github.io/nanoFORTH/html/page3.html" _target="_blank">References and all the details...</a>

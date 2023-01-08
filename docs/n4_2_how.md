\page page2 .2 How

Start wiring up your breadboard and ready to have some fun? Well, since FORTH is such a different language, a better preparation is always recommended...

### An Interactive FORTH Tutorial
If your programming language exposure has been with C, Java, or even Python so far, FORTH is quite **different**. Quote Nick: <em>"It's no functional or object oriented, it doesn't have type-checking, and it basically has zero syntax"</em>. No syntax? So, anyway, before you dive right into the deep-end, here's a good online materials.
* Interactive tutorial for FORTH primer. It teaches you how FORTH fundamentally works such as the numbers, the stack, and the dictionary.
> <a href="https://skilldrick.github.io/easyforth/#introduction" target="_blank">Easy Forth Tutorial by Nick Morgan</a>

You can skip to next section and start getting your hands dirty right away. However, if you prefer gathering all the sticks before starting a fire or enjoy immersing yourself in the philosophical wonderland of FORTH, excellent online e-books are here free for you.
* Timeless classic for the history, paths, and thoughts behind FORTH language.
> <a href="http://home.iae.nl/users/mhx/sf.html" target="_blank">Starting Forth by Leo Brodie</a><br/>
> <a href="http://thinking-forth.sourceforge.net" target="_blank">Thinking Forth by Leo Brodie</a>

### Install nanoFORTH - to be simple and useful

* From Arduino IDE's Library Manager
> <br/>
> \> Make sure you've hooked up one of Arduino Nano/Uno, or a development board that hosts ATmega328
>
> \> from Arduino IDE > Tools > Manage Libraries, enter FORTH in search box
>
> \> find nanoFORTH in the short list, select the latest version, and click the Install button
>
> \> from Files > Examples, find nanoForth in Examples from Custom Libraries at very buttom section
>
> \> load one of the nanoForth examples, such as 0_blink
>
> \> open Serial Monitor, set baud rate to 115200, and line ending to 'Both NL & CR'
>
> \> hit compile and upload. You should see the 'ok' prompt, and the built-in LED should be blinking
>
> \> in Serial Monitor input bar atop, type WRD and hit <return>. See what nanoFORTH says.<br/><br/>

* or, from GitHub directly, if you prefer managing source codes manually
> <br/>
> \> git clone https://github.com/chochain/nanoFORTH onto your local Sketch directory
>
> \> copy examples/0_blink/0_blink.ino from sub-directory, then rename it as nanoFORTH.ino<br/>
>
> \> open nanoFORTH.ino with Arduino IDE, and setup your Nano/Uno (or ATmega328) development board
>
> \> in nanoFORTH.ino, change the #include <nanoforth.h> to #include "./src/nanoforth.h"
>
> \> open Serial Monitor, set baud rate to 115200, and line ending to 'Both NL & CR'
>
> \> compile and upload, you should see the 'ok' prompt, and the built-in LED is blinking
>
> \> in Serial Monitor input bar atop, type WRD and hit <return>. See what nanoFORTH says.<br/><br/>

* Hopefully, thing goes well and you get something like this if nanoFORTH is uploaded successfully 
> <br/>
> |screen shot|
> |:--|
> |@image html images/nanoforth_init_screen.png|
<br/>

### Exercise

We have gone through a lot of 'paper work', time for some hands-on again. If needed, or you've skipped through somehow, please do review the previous page <a href="page1.html" target="_blank">HERE (the Why page)</a> for some instructions we've gone through earlier.

Now let's try some fancy stuffs to see what nanoFORTH has to offer.
* turn the tracing flag on, and try everything we did in the previous page
> 1 TRC ⏎<br/>
> **lit** **?Z**

* if it's too much info for you, then turn the tracing off
> 0 TRC ⏎<br/>

* to do some logic ops in hex, and back in decimal, we can do this
> HEX ⏎<br/>
> 40 38
> ⇨ 40_38_ok
> OR ⏎<br/>
> 78_ok
> DEC ⏎<br/>
> ⇨ 120_ok

* get Arduino clock/millis, a double precision (i.e. 32-bit) value
> CLK ⏎<br/>
> ⇨ 17786_9_ok
>> \> nanoFORTH uses two 16-bit cells on data stack to represent the 32-bit number<br/>
>> \> note the numbers above are for example only, your clock read will be different

* to benchmark something, let's define a function **zz** that runs in empty loops and time it
> : **zz** 10000 FOR NXT ;⏎<br/>
> CLK DNG **zz** CLK D+ ⏎<br/>
> ⇨ 160_0_ok
>> \> Our ten-thousand cycles are completed in 160ms, i.e. 16us/cycle, not too shabby!<br/>
>> \> DNG negate the first clock ticks<br/>
>> \> D+ add two clock counts (i.e. (-t0) + t1) to deduce the time difference

* to find out how many bytes of memory has been used
> HRE ⏎<br/>
> ⇨ 76_ok

* to dump the memory to see how all these words are encoded in the dictionary
> 0 HRE DMP ⏎
>> \> There! You can see the hex dump of our **red** ... **blu** ...<br/>
>> \> in their gory detail all the way up to the latest word **zz**

* at the end of the day, or to prevent a power outage hit, we can save what's been done so far into EEPROM
> SAV ⏎

* when needed, we can zap the sandbox i.e. reset the nanoFORTH system pointers for a fresh start
> BYE ⏎<br/>
> ⇨ nanoFORTH v1.6 ok
>> \> The data stack, return stack, and instruction pointers will be reinitialized

* after restart your Arduino, words can be restored from EEPROM where you saved earlier.
> LD ⏎<br/>
> 0 HRE DMP ⏎

Alright! That has pretty much concluded our rounds of exercise. You probably have wondered that if the SAV/LD pair can provide the ability to withstand power failures, can it be rebooted in the field for our future apps on Nanos? Well, yes! To do that, we have another word for you - **SEX** - it is! Short for Save and Execute. It saves the dictionary into EEPROM and set the autorun flag. When your Arduino reboots, the flag in EEPROM is checked. If it is indeed set, the last word saved will be executed.
* here's one example
> : **fun** ( - - ) 1000 DLY ." I'm alive! blink " 20 **xy** ; ⏎<br/>
> SEX ⏎<br/>
> BYE ⏎<br/>
> ⇨ nanoFORTH v1.6 reset<br/>
> ⇨ I'm alive! blink 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 ok<br/>
>> \> When you entered BYE this time, nanoFORTH reboot and runs the last word you've saved. In our case, it is **fun**, the blinker.<br/>
>> \> Note that the ( - - ) is a Forth-style comment that you can use. A \\ (back slash) can also be used to ignore comments to the end of your input line.<br/>
>> \> if you missed how **xy** worked, you might need to revisit the previous page <a href="page1.html" target="_blank">HERE</a> 

* to disable the autorun, a normal SAV again will clear the flag. It does keep your dictionary intact in EEPROM, i.e. words you've created before are still in place.
> SAV ⏎<br/>
> BYE ⏎<br/>
> ⇨ nanoFORTH v1.6 ok

OK, we know microcontrollers in the field are often built to run in an endless loop. However, before you get creative and save the wonderful service routine into EEPROM, I have to confess that I actually do not know how to get out of a reboot loop yet. Since it might be your last word, double check it. Any suggestion is welcome before people hitting that button and stuck forever.

So, nanoFORTH is **real-time**, and can **multi-task**. It is **interactive** and **extensible**. It can be reprogrammed on-the-fly or even over-the-air. Many many exciting stuffs can be added onto this simple system. Hopefully, this is a start of a fun journey far and beyond.

<br/>
<a href="page1.html">1. Why - Review nanoFORTH command examples</a><br/>
<a href="page3.html">3. What - Learn nanoFORTH internal details</a>




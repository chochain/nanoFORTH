\page page2 How

Start wiring up your breadboard and ready to have fun? Well, since FORTH is such a different language, a better preparation is always recommended...

### An Interactive FORTH Tutorial
Since FORTH is **different** if your exposure has been with C, Java, or even Python so far. Quote Nick: <em>"It's no functional or object oriented, it doesn't have type-checking, and it basically has zero syntax"</em>. No syntax? So, anyway, before you dive right into the deep-end, here's a good online materials.
* Interactive tutorial for FORTH primer. It teaches you how FORTH fundamentally works such as the numbers, the stack, and the dictionary.
> <a href="https://skilldrick.github.io/easyforth/#introduction" target="_blank">Easy Forth Tutorial by Nick Morgan</a>

You can skip to next section and start getting your hands dirty right away. However, if you prefer gethering all the sticks before starting a fire or enjoy imensing yourself in the philosophical wonder of FORTH, excellent online e-books are here free for you.
* Timeless classic for the history, paths, and thoughts behind FORTH language.
> <a href="http://home.iae.nl/users/mhx/sf.html" target="_blank">Starting Forth by Leo Brodie</a><br/>
> <a href="http://thinking-forth.sourceforge.net" target="_blank">Thinking Forth by Leo Brodie</a>

### Install nanoFORTH - to be simple and useful

* From GitHub directly
> <br/>
> \> git clone https://github.com/chochain/nanoFORTH onto your local Sketch directory
>
> \> copy examples/0_blink.ino from sub-directory, rename it as nanoFORTH.ino<br/>
>
> \> open nanoFORTH.ino with Arduino IDE, setup for your Nano/Uno development board
>
> \> open Serial Monitor, set baud rate to 115200, line ending to 'Both NL & CR'
>
> \> compile and upload, you should see the 'ok' prompt, and the built-in LED is blinking
>
> \> in Serial Monitor input bar atop, type WRD and hit return. See what nanoFORTH says.<br/><br/>

* From Arduino Library Manager
> <br/>
> \> from Arduino IDE->Tools->Manage Libraries, enter FORTH in search box
>
> \> find nanoFORTH in the short list, select latest version, click Install button
>
> \> click the <a href="http://github.com/chochain/nanoFORTH" target="_blank">More Info</a> link taking you to nanoFORTH Github site
>
> \> copy examples/0_blink.ino to your own Sketch directory, rename it accordingly
>
> \> open Serial Monitor, set baud rate to 115200, line ending to 'Both NL & CR'
>
> \> compile and upload, you should see the 'ok' prompt, and the built-in LED is blinking
>
> \> in Serial Monitor input bar atop, type WRD and hit return. See what nanoFORTH says.<br/><br/>

* Serial Monitor screenshot (sample) once nanoFORTH is successfully uploaded
> <br/>
> |screen shot|
> |:--|
> |@image html nanoforth_init_screen.png|
<br/>

### Exercise

We have gone through a lot of 'paper work', time for hands-on again. If needed, review previous page <a href="page1.html" target="_blank">here</a> for some instructions we've gone through earlier.

Now let's try some fancy stuffs to see what nanoFORTH has to offer.
* turn the tracing flag on, you can and try everything we did in privious page
> 1 TRC ⏎<br/>
> **lit** **?Z**

* too much info, you can turn the tracing off
> 0 TRC ⏎<br/>

* get Arduino clock/millis, a double precision (i.e. 32-bit) value
> CLK ⏎<br/>
> -26395_188_ok (for example only, your clock is different)<br/>
>> \> we need two 16-bit cells on data stack to represent the double

* to benchmark something, let's defined an empty loop and time it
> : **zz** 10000 0 FOR NXT ;⏎<br/>
> CLK DNG **zz** CLK D+ ⏎<br/>
> 338_0_ok (our ten-thousand cycles are completed in 338ms, i.e. 34us/cycle, not bad!)<br/>
>> \> DNG negate the first clock ticks<br/>
>> \> D+ add two clock counts (i.e. (-t0) + t1) to deduce the time difference

* find out how many bytes of memory we have used
> HRE ⏎<br/>
> 76_ok

* dump the memory to see how all these words are encoded in the dictionary
> 0 HRE DMP ⏎

* at the end of day, or in case of a power outage, let's save what's been done so far into EEPROM
> SAV ⏎

* when needed, you can clean up the playground i.e. reset nanoFORTH system pointers for a fresh start
> BYE ⏎<br/>
> nanoFORTH v1.0 ok

* after restart your Arduino, words can be restored from EEPROM which you saved earlier.
> LD ⏎<br/>
> 0 HRE DMP ⏎

Alright! That pretty much concluded our rounds of exercise. You probably have guessed that the SAV/LD pair can give our future Nano running in the field the ability to withstand power failures or reboots. Yes, indeed if we setup an init address properly.

It's real-time, and it multi-tasks. It can be reprogrammed on-the-fly or even over-the-air. It is extensible. Many many exciting stuffs can be added onto this simple system. Hopefully, this is a start of a fun journey far and beyond.

<br/>
<a href="page1.html">Review nanoFORTH command examples</a><br/>
<a href="page3.html">Learn nanoFORTH internal details</a>




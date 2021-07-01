\page page2 How

Start wiring up your breadboard and ready to have fun? Well, since FORTH is such a different language, a better preparation is always recommended...

### An Interactive FORTH Tutorial
Since FORTH is **different** if your exposure has been with C, Java, or even Python so far. Quote Nick: <em>"It's no functional or object oriented, it doesn't have type-checking, and it basically has zero syntax"</em>. No syntax? So, anyway, before you dive right into the deep-end, here's a good online materials.
* Interactive tutorial for FORTH primer. It teaches you how FORTH fundamentally works such as the numbers, the stack, and the dictionary.
> <a href="https://skilldrick.github.io/easyforth/#introduction" target="_blank">Easy Forth Tutorial by Nick Morgan</a>

You can skip to Installation Section and start getting your hands dirty. Jump right ahead to next section. However, if you prefer gethering all the sticks before starting a fire or enjoy imensing yourself in the philosophical wonder of FORTH, excellent online e-books are here free for you.
* Timeless classic for the history, paths, and thoughts behind FORTH language.
> <a href="http://home.iae.nl/users/mhx/sf.html" target="_blank">Starting Forth by Leo Brodie</a><br/>
> <a href="http://thinking-forth.sourceforge.net" target="_blank">Thinking Forth by Leo Brodie</a>

### Install nanoFORTH - to be simple and useful

* From GitHub directly
><br/>
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
><br/>
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


### Exercise

Review previous page <a href="page1.html" target="_blank">here</a> for same instructions we've gone through earlier.

Compared to any FORTH language tutorial, you probably will notice that length of a word of nanoFORTH, unlike most are 31-character, is 3 characters or less. This departs from standard FORTHs and begs the question of whether nanoFORTH is truly a FORTH. Well, our target platform is a very small MCU and our application has probably a dozen of functions. Aside from easier to type, it has benefit in simplifying some internal handling. The theory says that our brain is pretty good at filling the gap. So, hopefully, with a little bit creativity, our code can be clean and still maintainable. To qualify it as a FORTH or not, probably doesn't matter that much so long as it behaves well, runs fast enough, and useful for our needs.

Now let's try some fancy stuffs to see what nanoFORTH has to offer.
* turn the tracing flag on, you can and try everything we did in privious page
> 1 TRC ⏎<br/>
> **lit** **?Z**

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

* after restart your Arduino, words can be restored from EEPROM which you saved earlier
> LD ⏎<br/>
> 0 HRE DMP ⏎

<br/>
<a href="page1.html">Review nanoFORTH command examples</a><br/>
<a href="page3.html">Learn nanoFORTH internal details</a>




\page page2 What

Start wiring up your breadboard and ready to have fun? Well, since FORTH is such a different language, a better preparation is always recommended...

### An Interactive FORTH Tutorial
Since FORTH is quite a different language if your exposure so far has been pretty much C, Java, or even Python. Quote Nick: <em>"It's no functional or object oriented, it doesn't have type-checking, and it basically has zero syntax"</em>. So, before you dive right into the deep-end, here's a good online interactive tutorial for FORTH primer. It teaches you how FORTH fundamentally work such as the numbers, the stack, and the dictionary.
> <a href="https://skilldrick.github.io/easyforth/#introduction" target="_blank">Easy Forth Tutorial by Nick Morgan</a>
You can skip to Installation Section and start getting your hand dirty. However, if you prefer gethering all the sticks before starting a fire or enjoy imensing yourself in the philosophical beauty of FORTH, Leo Brodie's timeless classic are online free for you here.
> <a href="http://home.iae.nl/users/mhx/sf.html" target="_blank">Starting Forth by Leo Brodie</a><br>
> <a href="http://thinking-forth.sourceforge.net" target="_blank">Thinking Forth by Leo Brodie</a>

### Install nanoFORTH - to be simple and useful

todo: wait until Arduino forum accept the package

### Exercise

Right after reading any FORTH language tutorial, you will notice that words of nanoFORTH, unlike most are 31-character, are all 3 characters or less. For a start, I like to be brief. Our target platform is a very small MCU and our application has probably a dozen of functions. Aside from easier to read, it has benefit in simplifying some internal handling. The theory says that our brain is pretty good at filling the gap. So, hopefully, with a little bit creativity, our code can be clean and still maintainable.

* now some fancy stuffs, to see what nanoFORTH did, turn the tracing flag on and try everything we did in privious page
> 1 TRC<br>
> **lit** **?Z**

* find out how many bytes of memory we have used
> HRE<br>
> 76_ok

* dump the memory to see how all these words are encoded in the dictionary
> 0 HRE DMP

* at the end of day, or in case of a power outage, let's save what's been done so far into EEPROM
> SAV

* when needed, you can clean up the playground i.e. reset nanoFORTH system pointers for a fresh start
> BYE<br>
> nanoFORTH v1.0 ok

* after restart your Arduino, words can be restored from EEPROM which you saved earlier
> LD<br>
> 0 HRE DMP

<br>
<a href="page3.html">To learn nanoFORTH internal details...</a>




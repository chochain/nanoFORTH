# Journey of an Arduino Maker

**You unboxed the Arduino UNO** from Amazon with excitement, opened the awaited Arduino IDE, carefully followed one of the numerous available online tutorials, compiled the Blink sketch from the included examples, and hit the -> button. A few seconds later, the orange light started blinking! Success! You told yourself that "I think I can be pretty good at this"! ...<a href="https://chochain.github.io/nanoFORTH/html/page2.html" target="_blank">TL;DR, click here, skip to installation</a>

**What comes next** is weeks of ego-boosting new component badges you collected. From the simple humidity-temperature (DHT), ultrasound (HC-SR04), to how-do-I-read-the-resistor-code LEDs, why NEC infrared remote (HX1838), flimsy SG90 servos, and finally the damn gyro (MPU6050). OK, that was a pretty good run, you thought. Kids were fascinated, and even my typically unconcerned wife seemed to be impressed.

**three more months has gone by**. Hundreds of compilation/uploading later, though your faithful UNO runs flawlessly blinking all the LEDs you gave him, somewhere in the back of your mind the flash memory's 100K write cycle thing arises. Maybe it's time to give the little brother, Nano, a try. Well, while we're at it, for $20, why not get the pack of five package.

**So, our journey started.** A long time ago in the digital galaxy far, far away....a simple yet extensible programming language, FORTH, was often found on memory constrained space crafts or remote probes. With the ability to not only compile and run given code but also interact with the operator via a simple serial console. In today's Earth term, it's a complete REPL programming environment that can be controlled over wireless connection.

**Jump back to our Nanos**. The same old cycles of your C coding in the Arduino IDE. Compile/upload, compile/upload,... until you're more or less ran out of ideas. Of the 32K flash equipped with Nano, you have never once used over 10% and occasionally wonder how many lines of code it takes to fill it up. Over Google, some said 20% with over 30K lines of code. However, they also said, to do anything more meaningful, the 2K RAM will quickly became the limiting factor. You hop from example project to projects, wondered in the empty space for a long while. Eventually, a sense of lost took over, you went back to watching Netflix and the once vibrant mind sunk into the daily mundane.

One day, **ESP shows up** on you radar. It can do everything you've imagined thus far. Web serving, mesh-network, LoRa, even MySQL,... Programming-wise, there are C, microPython, even Ruby after some hard digging. Soon, the universe started to expand and what came into view is a Raspberry Pi. The final frontier suddenly became boundless. Your imagination was pushed far beyond the black holes and the vast void. **Life became meaningful** and sure got excited again. MotionEyes, OctaPi, robot buggy, and the plan to try some AI stuffs, ... Your heart were filled with joy and temptations. The limitation of CPU resource or storage is already a distant memory.

**One night**, while you felt the new path is set and the final destination is in sight, you were driving home with the cresting moon hanging over the horizon, a thought came through your mind. **What happened to the Nanos?**
<br/>
<br/>
<br/>
***
### Flash backs...

Compiling code as Sketches in the IDE and Upload via the tethered USB cable has been the way of life in Arduino universe. Makers are used to getting feedback directly from Serial Monitor as well. This self-contained round-trip development cycle from the comfort of all inside one IDE is the major reason making this platform so popular.

FORTH, a simple yet extensible interactive language, is the only programming language that runs and compiles on the ATmega328 so far. Arms embedded platforms with REPL coding/debugging process makes it a natural candidate for micro-controllers. Its interactive shell can eliminate the bulk of the repetitive compile/upload cycles.

Currently, two popular implementations, **AmForth**, **FlashForth**, and a lesser known **328eForth** are available for Arduino Uno. Though no direct support from the IDE yet, they have demonstrated the value of such on tiny systems. However, in order to run a simplest command, all of them are **required to overwrite the Arduino bootloader** which also needs an additional burner (or called programmer). The additional process not only is an entry barrier for beginners but render your kit a 'non-Arduino' which can leave a "your warranty is void" taste in your mouth even when done correctly. A frequently asked question is "how do I turn it back?"

Alternatively, **YAFFA** (Yet-Another-Forth-For-Arduino), can be pulled from GitHub directly and compiled in IDE as a Sketch with just an extra download of a memory library. It can be fitted tightly onto Uno/Nano and do adhere to standard FORTH syntax. The complete vocabulary of 150+ words can be a little overwhelming for beginners but should suit veterans well who are already comfortable with FORTH. For development, with not much free memory left for you, it might be **a bit constrained**. Though it certainly is done nicely if you prefer the feel of what a "normal" FORTH does. However, not been able to save your work (say in EEPROM) after the micro-controller restart, it can seriously limit the usability. Finally, if you are interested in how FORTH can be cleanly coded in C++, plus with tons of macros, there is **FVM** (Forth-Virtual-Machine). It can be compiled directly using Arduino IDE as well. Though not as complete, the extensibility and **fast multi-tasking** capabilities can teach us a great lesson. Aside from the few entries mentioned above, a search on Goggle does not yield a lot more on the subject. So, there you have it.

May the FORTH be with you! Here comes **a simple and useful** one for our **Nanos**!

<br/>
<a href="https://chochain.github.io/nanoFORTH/html/page1.html">[Buckle up! Ready for Hyper Jump...]</a>



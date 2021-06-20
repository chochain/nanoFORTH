# Journey of an Arduino Maker

**You unboxed the Arduino UNO** from Amazon with excitement, opened the awaited Arduino IDE, carefully followed one of the numerous available online tutorials, dug and compiled the Blink sketch from the included examples, hit the -> button and seconds later, the orange light started blinking! Success! You told yourself that "I think I can be pretty good at this"!

**What comes next** is weeks of ego-boosting of new component badges you collected. From the simple humidity-temperature (DHT), ultrasound (HC-SR04), to how-do-i-read-the-resistor-code LEDs, why-NEC infrared remote (HX1838), flimsy SG90 servos, and finally the damn gyro (MPU6050). OK, that was a pretty good run, you thought. Kids were facinated, and even my typically unconcerning wife seemed to be impressed.

**3 more months has gone by**. Hundreds of compilation/uploading later, though your faithful UNO runs flawlessly blinking all the LEDs you gave him, somewhere in the back of your mind the flash memory's 100K write cycle thing arised. Maybe it's time to give the little brother, Nano, a try. Well, while we're at it, for $20, lets get a pack of five of these little guys might as well.

**So, our journey started.** A long time ago in the digital galaxy far, far away....a simple yet extensible programming language, FORTH, was often found on memory constrained space crafts or remote probes. With the ability to not only compile and run given code but also interact with the operator via a simple serial console. In Earth today's term, it's a complete REPL programming environment, and possiblly can be controlled via wireless connection.

**Jump back to our Nanos**. The same old cycles of your C coding in the Arduino IDE. Compile/upload, compile/upload,... until you're more or less ran out of ideas. Of the 32K flash equiped with Nano, you have never used over 10% and ocassically wonder how many lines of code it takes to fill it up. Over Google, some said 20% with over 30K lines of code. However, they also said, to do anything more meaningful, the 2K RAM will quickly becomes the limiting factor. You hop from example project to projects, wonder in the empty space for a long while. Eventually, feeling sense of lost, your went back to watching Netflex and the once vibrant mind sunk into the daily mondane.

One day, **ESP shows up** on you radar. It can do everything you've imagined thus far. Web serving, mesh-network, LoRa, even MySQL,... Programming-wise, there're C, microPython, even Ruby after some hard digging. Soon, the universe started to expand and what came into view is a Raspberry Pi. The final frontier suddenly became bondless. Your imagination was pushed far beyond the black holes and the vast void. **Life became meaningful** and sure got excited again. MotionEyes, OctaPi, robot buggy, and the plan to try some AI stuffs, ... Your heart were filled with joy and temptations. The limitation of CPU resource or storage is already a distant memory.

**One night**, while you felt the new path is set and the final destination is in sight, you were driving home with the cresting moon hanging over the horizon, a thought came through your mine. **What happened to the Nanos?**
<br>
<br>
<br>
***
### Flash backs...

Compling code as Scetches in the IDE and Upload via the tethered USB cable has been the way of life in Arduino universe. Makers are used to getting feedback directly from Serial Monitor as well. This self-contained round-trip development cycle from the comfort of all inside one IDE is the major reason making this platform so popular.

FORTH, a simple yet extensible interactive language, is the only programming language that runs and compiles on the ATmega328 so far. Arms embedded platforms with REPL coding/debugging process makes it a natual candidate for microcollers. Its interactive shell can eliminate the bulk of the repetitive compile/upload cycles.

Currenly, two popular implementations, **AmForth**, **FlashForth**, and a lesser known **328eForth** are avaiable for Arduino Uno. Though no direct support from the IDE yet, they demonstrated the value of such on tiny systems. However, in order to run a simplest command, all of them are **required to overwrite the Arduino bootloader** whcich also needs an additional burner (or called programmer). The additional process not only is an entry barier for beginners but render your kit a 'non-Arduino' which can leaves a "your warrenty is void" taste in your mouth even if done correctly. An often asked question is "how do I turn it back?".

Alternatively, **YAFFA** (Yet-Another-Forth-For-Arduino), can be pulled from GitHub directly and compiled in IDE as a Sketch with just an extra download of a memory library. It can fit tightly onto Uno/Nano and do adhere to standard FORTH synatx. The complete 150+ word vocabulary can be a little overwhelming for beginners but should suit veterans well who are comfortable with FORTH. For development, it might be **a bit constrained** with available memory left free for you. Though certainly is done nicely if you like the feel of what a "normal" FORTH does but not been able to save your work (say in EEPROM) after restart can seriously limit its usability. Finally, if you are interested in how cleanly FORTH can be coded in C++ with tons of macros, there is **FVM** (Forth-Virtual-Machine). It can be compiled directly using Arduino IDE as well. Though not as complete, the extensibility and **fast multi-tasking** capabilities can teach us a great lesson. Aside from the few entries mentioned above, a search on Goggle does not yield a lot more on the subject. So, there you have it.

May the FORTH be with you! Here comes **a simple and useful** one for our **Nanos**!

<a href="html/page1.html">[Buckle up! Ready for Hyper Jump...]</a>



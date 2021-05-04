# nanoFORTH
FORTH for Arduino Nano (and UNO), implemented in C

##
You unboxed the Arduino UNO from Amazon with excitement, opened the awaiting Arduino IDE, carefully followed one of the numerous available online tutorials, found and compiled the Blink sketch in the included examples, hit the -> button and seconds later, the orange light started blinking! Success! You told yourself that "I think I can be pretty good at this"!
##
What comes next is weeks of ego-boosting of new component badges you collected. From the simple humidity-temperature (DHT), ultrasound (HC-SR04), to how-do-i-read-the-resistor-code LEDs, why-NEC infrared remote (HX1838), flimsy SG90 servos, and finally the damn gyro (MPU6050). OK, that was a pretty good run, you thought. Kids were facinated, and even my typically unconcerning wife seemed to be impressed.
##
3 more months has gone by. Hundreds of compilation/uploading later, though your faithful UNO runs flawlessly blinking all the LEDs you gave him, somewhere in the back of your mind the flash memory's 100K write cycle thing arised. Maybe it's time to give the little brother, Nano, a try. Well, while we're at it, for $20, lets get a pack of five of these little guys might as well.
##
So, our journey started. A long time ago in the digital galaxy far, far away....a simple yet extensible programming language, FORTH, was often found on memory constrained space crafts or remote probes. With the ability to not only compile and run given code but also interact with the operator via a simple serial console. In Earth today's term, it's a complete REPL programming environment, and possiblly can be controlled via wireless connection.
##
Jump back to our Nanos. The same old cycles of your C coding in the Arduino IDE. Compile/upload, compile/upload,... until you're more or less ran out of ideas. Of the 32K flash equiped with Nano, you have never used over 10% and ocassically wonder how many lines of code it takes to fill it up. Over Google, some said 20% with over 30K lines of code. However, they also said, to do anything more meaningful, the 2K RAM will quickly becomes the limiting factor. You wonder in the space for a long while. Feeling sense of lost and your mind eventually went back to daily mondane.
##
One day, ESP shows up on you radar. It can do everything you've imagined thus far. Web serving, mesh-network, LoRa, even MySQL,... Programming-wise, there're C, microPython, even Ruby after some hard digging. Soon, the universe started to expand and what came into view is a Raspberry Pi. The final frontier suddenly became bondless. Your imagination was pushed far beyond the black holes and the vast void. Life became meaningful and sure got excited again. MotionEyes, OctaPi, robot buggy, and the plan to try some AI stuffs, ... Your heart were filled with joy and temptations. The limitation of CPU resource or storage is already a distant memory.
##
One night, while you felt the new path is set and the final destination is in sight, you were driving home with distant cresting moon hanging, a thought flowed through your head. What about the Nanos?
##
Compling code in the IDE and upload directory via the tethered USB cable has become the way of life in Arduino universe. Makers can also get feedback directly from Serial Monitor. This self-contained two-way development cycle from within the IDE is the major reason making this platform so popular. FORTH, arms the platform with REPL that simplify coding/debugging process, makes it a natual choice for microprocessor. Currenly, there are AmForth and FlashForth avaiable for Arduino. However, both of them require overwrite Arduino bootloader whcich requires not only an additional burner but also render your kit a non-Arduino. This is a mental challenge for beginners and will leaves a taste of "your warrenty is void" in your mouth if done.
##
So, the requirements for nanoFORTH are
* be as simple as any example Sketch that comes with the IDE (no bootloader burning)
* provide a REPL development/operating environment for Arduino
* support core Arduino functions (i.g. pinMode, digitalRead,Write, analogRead,Write, millis, delay)
* provide a FORTH thread and at least one separate hardware thread for additional hardware functionalities
* provide at least 1K RAM dictionary for resonablly size of work
* provide EEPROM persisted storage for new words which can be automatically reloaded on next start-up
* optionally show byte-code stream while assembled to help beginers understand FORTH internal
* optionally show execution tracing to help debugging, also provision for single-stepping


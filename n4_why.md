\page page1 1 Why

Compling code as Scetches in the IDE and Upload via the tethered USB cable has been the way of life in Arduino universe. Makers are used to getting feedback directly from Serial Monitor as well. This self-contained round-trip development cycle from the comfort of all inside one IDE is the major reason making this platform so popular.

FORTH, a simple yet extensible interactive language, arms embedded platforms with REPL coding/debugging process, makes it a natual candidate for microcollers. Its interactive shell can eliminate the bulk of the repetitive compile/upload cycles.

Currenly, there are AmForth and FlashForth avaiable for Arduino. Though no direct support from the IDE yet, they demonstrated the value of such on tiny systems. However, both of them also required to overwrite Arduino bootloader whcich needs an additional burner (or called programmer). The additional process not only is an entry barier for beginners but also render your kit a 'non-Arduino' which can leaves a "your warrenty is void" taste in your mouth even if done correctly. An often asked question is "how do I turn it back?".

So, may the FORTH be with you, and here comes one for our **Nanos**!

***

## nanoFORTH
### With the following assumptions for our Nanos,
* more than 80% of Arduino makers are using UNO or Nano,
* most of the makers do not need the full blown FORTH vocabularies,
* most of them are not familer with standard FORTH words, so abbriviation for words is OK,
* the meta-compiler is unlikely needed either, i.e. not to create a new type of Forth from within nanoForth,
* only a small set of core primitive words are needed for the most of Arduino projects,
  the rational being anything that requires more complicated syntax, one might need the power of ESP.

### The requirements for nanoFORTH are:
* be as simple to use as any example Sketch that comes with the IDE (no bootloader burning),
* provide a REPL development/operating environment for Arduino,
* provide core Arduino functions (i.g. pinMode, digitalRead,Write, analogRead,Write, millis, delay),
* provide hardware thread(s) in addition to nanoFORTH thread so new components can be added (ig. Bluetooth),
* provide at least 1K RAM dictionary for resonablly size of work,
* provide EEPROM persisted storage for new words which can be automatically reloaded on next start-up,
* optionally show byte-code stream while assembled to help beginers understand FORTH internal,
* optionally show execution tracing to help debugging, also provision for single-stepping,
* optionally implemented as an Arduino library that developers can include easily.


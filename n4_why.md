\page page3

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


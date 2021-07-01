\page page3 What

## nanoFORTH Operations
### Internal
At its core, nanoFORTH is a traditional **parse-dispatch virtual machine** interpreter with twin stacks. Essentially, it is a parser and a big switch loop. Like every other FORTH, it has dual personallities which toggles between interpreter mode and compiler mode. In interpreter mode, it runs interactively not unlike an old HP hand-held calculator. When in compiler mode, it transcodes user defined functions into <a href="https://www.complang.tuwien.ac.at/forth/threaded-code.html#what" target="_blank">token threaded code</a> which is faster and **portable**. The later ability enables future units to send data packets as well as instruction code segments to each other. This can bring a big grin but, of course, we later might need to deal with the red flag raised by security concerning parties.

Compared to any FORTH language tutorial, you probably will notice that the length of a word of nanoFORTH, unlike most are 31-character, is 3 characters or less. This departs from standard FORTHs and begs the question of whether nanoFORTH is truly a FORTH. Well, our target platform is a very small MCU and our application has probably a dozen of functions. Aside from easier to type, it has benefit in simplifying some internal handling. The theory says that our brain is pretty good at filling the gap. So, hopefully, with a little bit creativity, our code can be clean and still maintainable. To qualify it as a FORTH or not, probably doesn't matter that much so long as it behaves well, runs fast enough, and useful for our needs.

### Arduino Nano Memory Map
TODO

## Built-in Words (TODO more)
### Arithmatics Ops
\+ - * / MOD NEG
> 17 5 + ⏎
>> 22_ok

> 1 2 3 4 + + + + ⏎
>> 10_ok

> 10 3 / ⏎
>> 3_ok

### Stack Ops
DRP DUP SWP OVR ROT

### Bit-wise Ops
AND OR XOR

### Logical Ops
NOT = < > <= >= <>

### Flow Control (Compiler mode)
IF ELS THN BGN UTL WHL RPT FOR NXT I
> f IF...THN<br/>
> f IF...ELS...THN<br/>
> BGN...RPT<br/>
> BGN...f UTL<br/>
> BGN...f WHL...RPT<br/>
> n1 n2 FOR...NXT

### Definding Words (Interactive mode)
: ; VAR CST FGT

### Memory Access
@ ! C@ C! >R R>

### Console I/O
KEY EMT CR . .\"

### Dictionary
WRD HRE

### Array
CEL ALO

### EEPROM Access
SAV LD

### Arduino
CLK DLY PIN IN OUT AIN PWM

### 32-bit Arithmatic (for Arduino Clock mostly)
D+ D- DNG
> CLK 1000 DLY CLK D- DNG ⏎

### System, Debug/Tracing
BYE DMP TRC

## Opcode Formats
>    `branching : 11BB aaaa aaaa aaaa            (12-bit absolute address)`<br>
>    `primitive : 10oo oooo                      (6-bit, i.e. 64 primitives)`<br>
>    `3-byte lit: 1011 1111 snnn nnnn nnnn nnnn  bf xxxx xxxx (16-bit signed integer)`<br>
>    `1-byte lit: 0nnn nnnn                      (0..127)`<br>
>    `n-byte str: len, byte, byte, ...           (used in print str)`<br>

<br/>
<a href="page1.html">Review nanoFORTH command examples</a><br/>
<a href="page2.html">Install nanoFORTH</a><br>





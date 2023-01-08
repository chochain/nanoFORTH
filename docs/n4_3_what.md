\page page3 .3 What

## nanoFORTH Operations
### Internal
At its core, nanoFORTH is a traditional **parse-dispatch virtual machine** interpreter with twin stacks. Essentially, it is a parser and a big switch loop. Like every other FORTH, it has a dual-personality which toggles between interpreter mode and compiler mode. In interpreter mode, it runs interactively not unlike an old HP hand-held calculator. When in compiler mode, it transcodes user defined functions into <a href="https://www.complang.tuwien.ac.at/forth/threaded-code.html#what" target="_blank">token threaded code</a> which is compact and **portable**. The latter ability enables future units to exchange not only data packets but also instruction code segments to each other. This can bring a big grin but, of course, we later might need to deal with the red flag raised by security concerning parties.

Compared to any FORTH language tutorial, you probably will notice that the length of a word of nanoFORTH, unlike most are 31-character, is 3 characters or less. Core vocabulary is a short list which means makers need to define one if a more elaborated function is needed. There is no floating-point or meta-compiler supported. These depart from standard FORTHs and begs the question of whether nanoFORTH is truly a FORTH. Well, our target platform is a very small MCU and our application has probably a dozen of functions. Aside from saving a few bytes, it has the benefit in simplifying internal searching. The theory says that our brain is pretty good at filling the gap. So, hopefully, with a little bit creativity, our code can be clean and still maintainable. To qualify it as a FORTH or not, probably doesn't matter that much so long as it behaves well, runs fast enough, and useful for our needs.

### Arduino Nano Memory Map
> |address|object|growth|forth|
> |--:|:---:|:--:|:--:|
> |0x900|Arduino RAM max|_|.|
> |0x8f6|global/static variables|⇩|.|
> |...|Arduino heap|⇩|.|
> |...|Forth input buffer|⇧|X|
> |0x618| **return stack** |⇩|X|
> |...|0x100 shared space|_|X|
> |0x518| **data stack** |⇧|X|
> |...|user defined words|⇧|X|
> |0x1e8| **user dictionary** starts|⇧|X|
> |...|Arduino libraries|_|.|
> |0x100|Arduino RAM starts|_|.|
> |0x000|Arduino registers|_|.|

Of course, we still have the 1K Flash Memory sitting on the side which can save and reload the user dictionary when instructed.

## Number Representation
nanoFORTH handles only integer numbers.
* 16-bit integer range -32727 to 32726
* 32-bit double  can be presented as two 16-bit numbers on data stack
* hex number can be input with **$** prefix

>
> **Examples**
>
> 20 ⏎ ➤ *20_ok*<br/>
> 10 $10 ⏎ ➤ *20_10_16_ok*<br/>

## Built-in Words
### Stack Ops
> |opcode|stack|description|
> |:--|:--|:--|
> |DRP|`(w -- )`|drop|
> |DUP|`(w -- w w)`|duplicate|
> |SWP|`(a b -- b a)`|swap|
> |OVR|`(a b -- a b a)`|over|
> |ROT|`(a b c -- b c a)`|rotate|
>
> **Examples**
>
> 20 10 ⏎ ➤ *20_10_ok*<br/>
> OVR ⏎ ➤ *20_10_20_ok*<br/>
> DRP ⏎ ➤ *20_10_ok*<br/>
> SWP ⏎ ➤ *10_20_ok*<br/>
> DUP ⏎ ➤ *10_20_20_ok*<br/>
> ROT ⏎ ➤ *20_20_10_ok*<br/>

### Arithmatics Ops
> |opcode|stack|description|
> |:--|:--|:--|
> |+  |`(a b -- a+b)`|add|
> |-  |`(a b -- a-b)`|subtract|
> |*  |`(a b -- a*b)`|multiply|
> |/  |`(a b -- a/b)`|divide|
> |MOD|`(a b -- a%%b`)|modulo|
> |NEG|`(a   -- -a)`|negate|
> |ABS|`(a   -- abs(a) )`|absolute value of a|
> |MIN|`(a b -- min(a, b) )`|minimum value between a and b|
> |MAX|`(a b -- max(a, b) )`|maximum value between a and b|
>
> **Examples**
>
> 17 5 + ⏎ ➤ *22_ok*<br/>
> 1 2 3 4 + + + ⏎ ➤ *10_ok*<br/>
> 10 3 / ⏎ ➤ *3_ok*<br/>

### Binary and Logical Ops
> |opcode|stack|description|
> |:--|:--|:--|
> |AND|(a b \- \- a&b)|binary and|
> |OR |(a b \- \- a\|b)|binary or|
> |XOR|(a b \- \- a^b)|binary xor|
> |NOT|`(a -- ^a)`|binary not|
> |LSH|`(n i -- n<<=i)`|left shift|
> |RSH|`(n i -- n>>=i)`|right shift|
> |= |`(a b -- a==b)`|equal|
> |< |`(a b -- a<b)`|less than|
> |> |`(a b -- a>b)`|greater than|
> |<>|`(a b -- a!=b)`|not equal|

### Word Definition and Dictionary Ops (in Interactive mode only)
> |opcode|stack|description|
> |:--|:--|:--|
> |:  |`( -- )`|start defining a new word|
> |;  |`( -- )`|end of word definition|
> |WRD|`( -- )`|list all words defined in nanoFORTH dictionaries|
> |HRE|`( -- w)`|get current user dictionary pointer|
> |FGT|`( -- )`|forget/remove functions|

### Flow Control (in Compiler mode only)
> |branching ops|description|
> |:--|:--|
> |f IF xxx THN|conditional branch|
> |f IF xxx ELS yyy THN|@image html images/forth_if_els_thn.gif width=300px|
> |BGN xxx f UTL|@image html images/forth_bgn_utl.gif width=300px|
> |BGN xxx f WHL yyy RPT|@image html images/forth_bgn_whl_rpt.gif width=300px|
> |n FOR xxx NXT|for loop, index value I count down from n to 1|

### Return Stack Ops
> |opcode|stack|description|
> |:--|:--|:--|
> |I |`( -- w)`|fetch word from top of return stack, aka R@ in other FORTHs|
> |>R|`(w -- )`|push word on top of data stack onto return stack|
> |R>|`( -- w)`| pop top of return stack value and push it onto data stack|
> * note: FORTH programmers often use return stack as temp storage. However do use >R and R> carefully and in Compile mode only or you risk messing up call depth which can crash FORTH interpreter.

### Memory Access Ops
> |opcode|stack|description|
> |:--|:--|:--|
> |\@ |`(a -- w)`|fetch a 16-bit value from memory address 'a'|
> |!  |`(a w -- )`|store a 16-bit value to memory address 'a'|
> |C\@|`(a -- w)`|fetch a single byte from memory address 'a'|
> |C! |`(a w -- )`|store a byte (or lower byte of the word) to memory address 'a'|
> * note: the above opcodes read/write nanoFORTH memory space directly. It provides the power to peek and poke random memory but also to shoot yourself on the foot. Use with caution.
>
> **Examples**
> see next section

### Variable, Constant, and Array Ops
> |opcode|stack|description|
> |:--|:--|:--|
> |VAR|`( -- )`|define a 16-bit variable|
> |CST|`(w -- )`|define a 16-bit constant|
> |ALO|`(w -- )`|allocate space on user dictionary (for array allocation)|
> |CEL|`( -- w)`|get number of byte of a single cell (for allocation)|
>
> **Examples**
>
> VAR **x** ➤ *ok* (a variable **x** is created on user dictionary)<br/>
> 3 **x** ! ➤ *ok* (store 3 into variable **x**)<br/>
> **x** @ 5 + ➤ *8_ok* (fetch value of **x** add 5 to it)<br/>
>
> 32 CST **N** ⏎ ➤ *ok* (a const **N** is created on user dictionary)<br/>
> **N** 1 + ⏎ ➤ *33_ok*<br/>
>
> VAR **z** 3 CEL ALO ⏎ ➤ *ok* (a variable **z** with 3 extra cells allocated, i.e. **z**[0..3])<br/>
> 5 **z** 2 CEL + ! ⏎ ➤ *ok*  (5 is stored into **z**[2])<br/>
> **z** 2 CEL + @ ⏎ ➤ *5_ok* (retrieve **z**[2] onto data stack)<br/>

### Console I/O
> |opcode|stack|description|
> |:--|:--|:--|
> |KEY |`( -- c)`|get a byte from input console|
> |EMT |`(c -- )`|write a byte to output console|
> |CR  |`( -- )` |send a \<return\> to console|
> |.   |`(w -- )`|print the value on data stack to output console|
> |.\" |( \- \- )|send the following string (terminated with a \") to output console|
>
> **Examples**
>
> : **hi** FOR ." hello!" 33 EMT CR NXT ; ⏎ ➤ *ok* (**hi** is now defined in user dictionary)<br/>
> 3 **hi** ⏎<br/>
> ➤ *hello!*<br/>
> ➤ *hello!*<br/>
> ➤ *hello!ok*

### Reset, Debug, and Tracing
> |opcode|stack|description|
> |:--|:--|:--|
> |RST|`( -- )`|reset nanoFORTH for debugging on PC|
> |BYE|`( -- )`|reset nanoFORTH on Arduino, exit to OS on other platform|
> |DMP|`(a w -- )`|dump nanoFORTH user dictionary from address 'a' for w bytes|
> |TRC|`(t -- )`|enable/disable execution tracing|
>
> **Examples**
> ||
> |:--|
> |@image html images/nanoforth_bye_trc_dmp.png width=800px|

### EEPROM Access
> |opcode|stack|description|
> |:--|:--|:--|
> |SAV|`( -- )`|save user dictionary into Arduino Flash Memory|
> |LD |`( -- )`|restore user dictionary from Arduino Flash Memory|
> |SEX|`( -- )`|SAV with autorun flag set in EEPROM for reboot/execution|

### Arduino Specific Ops
> |opcode|stack|description|
> |:--|:--|:--|
> |CLK|`( -- d)`|fetch Arduino millis() value onto data stack as a double number|
> |DLY|`(w -- )`|wait milliseconds (yield to hardware tasks)|
> |PIN|`(w p -- )`|pinMode(p, w)|
> |IN |`(p -- w)`|digitalRead(p)|
> |OUT|`(w p -- )`|digitalWrite(p, w)|
> |AIN|`(p -- w)`|analogRead(p)|
> |PWM|`(w p -- )`|analogWrite(p, w)|
> 
> **Examples**
>
> 1 13 OUT ⏎ ➤ *ok*  (built-in LED is turn on, i.e. digitalWrite(13, 1) called)<br/>

### Interrupt ops
> |opcode|stack|description|
> |:--|:--|:--|
> |TMR|`( n -- )`|set timer ISR with period at n*0.1 second i.g. 100 is 10 second|
> |PCI|`( p -- )`|capture pin #p change (either HIGH to LOW or LOW to HIGH)|
> |TME|`( f -- )`|enable/disable timer interrupt, 0:disable, 1:enable|
> |PCE|`( f -- )`|enable/disable pin change interrupt, 0:disable, 1:enable|
> Note: nanoForth utilizes timer2 for timer interrupt. It might conflict with libraries which also uses timer2 such as Tone().
>
> **Examples**
>
> : aa 65 emt ; ➤ *ok* (define a word **aa** which emit 'A' on console)<br/>
> : bb 66 emt ; ➤ *ok* (define a word **bb** which emit 'B' on console)<br/>
>
> 100 TMR **aa** ➤ *ok* (run **aa** every 10 seconds)<br/>
> 250 TMR **bb** ➤ *ok* (run **bb** every 25 seconds)<br/>
> 1 TME ➤ *ok* (enable timer interrupt)<br/>
> AABAAABAABAA (interrupt routines been called)<br/>
> 0 TME ➤ *ok* (disable timer interrupt)<br/>
>
> 8 PCI **aa** ➤ *ok* (run **aa** when pic 8 changed)<br/>
> 1 PCI ➤ *ok* (enable pin change interrupt)<br/>
> AA (assuming you have a push button hooked up at pin 8)<br/>
>

### 32-bit Arithmatic (for Arduino Clock mostly)
> |opcode|stack|description|
> |:--|:--|:--|
> |D+ |`(d1 d0 -- d1+d0)`|add two doubles|
> |D- |`(d1 d0 -- d1-d0)`|subtract two doubles|
> |DNG|`(d0 -- -d0)`|negate a double number|
>
> **Eexamples**
>
> CLK 1000 DLY CLK D- DNG ⏎ ➤ *1000_0_ok*

<br/>
## Function/Word Struct
> |size|field|
> |:--|:--|
> |fixed 16-bit|address to previous word, 0xffff is terminator|
> |fixed 3-byte|function name|
> |n-byte<br/>depends on the length of the function| * compiled opcodes (see next section), or<br/>* address of a user defined word|

## Opcode Memory Formats
> |opcode|stack|description|
> |:--|:--|:--|
> |1-byte literal|`0nnn nnnn`|0..127, often used, speeds up core|
> |3-byte literal|`1011 1111  snnn nnnn  nnnn nnnn`|16-bit signed integer|
> |1-byte primitive|`10oo oooo`|6-bit opcode i.e. 64 primitives|
> |branching opcodes|`11BB aaaa  aaaa aaaa`|12-bit address i.e. 4K space|
> |n-byte string|`len, byte, byte, ...`|256 bytes max, used in print string|

<br/>
<a href="page1.html">1. Why - Review nanoFORTH command examples</a><br/>
<a href="page2.html">2. How - Intall nanoFORTH</a>





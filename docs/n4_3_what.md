\page page3 What

## nanoFORTH Operations
### Internal
At its core, nanoFORTH is a traditional **parse-dispatch virtual machine** interpreter with twin stacks. Essentially, it is a parser and a big switch loop. Like every other FORTH, it has dual personallities which toggles between interpreter mode and compiler mode. In interpreter mode, it runs interactively not unlike an old HP hand-held calculator. When in compiler mode, it transcodes user defined functions into <a href="https://www.complang.tuwien.ac.at/forth/threaded-code.html#what" target="_blank">token threaded code</a> which is faster and **portable**. The later ability enables future units to send data packets as well as instruction code segments to each other. This can bring a big grin but, of course, we later might need to deal with the red flag raised by security concerning parties.

Compared to any FORTH language tutorial, you probably will notice that the length of a word of nanoFORTH, unlike most are 31-character, is 3 characters or less. This departs from standard FORTHs and begs the question of whether nanoFORTH is truly a FORTH. Well, our target platform is a very small MCU and our application has probably a dozen of functions. Aside from easier to type, it has benefit in simplifying some internal handling. The theory says that our brain is pretty good at filling the gap. So, hopefully, with a little bit creativity, our code can be clean and still maintainable. To qualify it as a FORTH or not, probably doesn't matter that much so long as it behaves well, runs fast enough, and useful for our needs.

### Arduino Nano Memory Map
> |address|object|growth|forth|
> |--:|:---:|:--:|:--:|
> |0x900|Arduino RAM max|_|.|
> |0x8f5|global/static variables|⇩|.|
> |...|free memory/heap|_|.|
> |0x696| **return stack** |⇩|X|
> |0x617| **data stack** |⇧|X|
> |...|user defined words|⇧|X|
> |0x217| **user dictionary** starts|⇧|X|
> |...|Arduino libraries|_|.|
> |0x100|Arduino RAM starts|_|.|
> |0x000|Arduino registers|_|.|

Of course, we still have the 1K Flash Memory sitting on the side which can save and reload the user dictionary when instructed.

## Nubmer Representation
nanoFORTH handles only integer numbers.
* 16-bit integer range -32727 to 32726
* 32-bit double  can be presented as two 16-bit numbers on data stack
* hex number can be input with **$** prefix 

#### examples
> 20 ⏎
>> 20_ok
> 10 $10 ⏎
>> 20_10_16_ok

## Built-in Words
### Stack Ops
* `DRP (w -- )`
* `DUP (w -- w w)`
* `SWP (a b -- b a)`
* `OVR (a b -- a b a)`
* `ROT (a b c -- b c a)`

#### examples
> 20 10 ⏎
>> 20_10_ok
> OVR ⏎
>> 20_10_20_ok
> DRP ⏎
>> 20_10_ok
> SWP ⏎
>> 10_20_ok
> DUP ⏎
>> 10_20_20_ok
> ROT ⏎
>> 20_20_10_ok

### Arithmatics Ops
* `+   (a b -- a+b)`
* `-   (a b -- a-b)`
* `*   (a b -- a*b)`
* `/   (a b -- a/b)`
* `MOD (a b -- a%%b)`
* `NEG (a   -- -a)`

#### examples
> 17 5 + ⏎
>> 22_ok
>
> 1 2 3 4 + + + + ⏎
>> 10_ok
>
> 10 3 / ⏎
>> 3_ok

### Bit-wise Ops
* `AND (a b -- a&b)`
* `OR  (a b -- a|b)`
* `XOR (a b -- a^b)`

### Logical Ops
* `NOT (a -- ^a)`
* `=   (a b -- a==b)`
* `<   (a b -- a<b)`
* `>   (a b -- a>b)`
* `<=  (a b -- a<=b)`
* `>=  (a b -- a>=b)`
* `<>  (a b -- a!=b)`

### Flow Control (in Compiler mode only)
* `f IF...THN`
* `f IF...ELS...THN`
* `BGN...f UTL`
* `BGN...f WHL...RPT`
* `BGN...f WHL...f UTL`
* `n1 n2 FOR...NXT`

### Definding Words (in Interactive mode only)
* `:   start defining a new word`
* `;   end of word definition`
* `VAR define a 16-bit variable`
* `CST define a 16-bit constant`
* `FGT forget/remove functions`

### Memory Access
* `@    (a -- w)`   fetch a 16-bit value from memory address 'a'
* `!    (a w -- )`  store a 16-bit value to memory address 'a'
* `C@   (a -- w)`   fetch a single byte from memory address 'a'
* `C!   (a w -- )`  store a byte (or lower byte of the word) to memory address 'a'

### Return Stack Ops
* `I    ( -- w)` fetch word from top of return stack
* `>R   (w -- )` push word on top of data stack onto return stack
* `R>   ( -- w)` pop top of return stack value and push it onto data stack

### Console I/O
* `KEY  ( -- c)` get a byte from input console
* `EMT  (w -- )` send a byte to output console
* `CR   ( -- )`  send a \<return\> to console
* `.    (w -- )` print the value on data stack to output console
* `."   ( -- )` print a string onto output console

### Dictionary
* `WRD  ( -- )`  list all words defined in nanoFORTH dictionaries
* `HRE  ( -- w)` get current user dictionary pointer

### Array
* `ALO  (w -- )` allocate space on user dictionary (for array allocation)
* `CEL  ( -- w)` get number of byte of a single cell (for allocation)

#### examples
> VAR x 2 CEL ALO ⏎
>> ok  (total 3 16-bit value allocated on user dictionary)

### EEPROM Access
* `SAV  ( -- )`  save user dictionary into Arduino Flash Memory
* `LD   ( -- )`  restore user dictionary from Arduino Flash Memory

### Arduino
* `CLK  ( -- d)` fetch Arduino millis() value onto data stack as a double number
* `DLY  (w -- )` wait milliseconds (yield to hardware tasks)
* `PIN  (w p -- )` call pinMode(p, w)
* `IN   (p -- w)`  call digitalRead(p)
* `OUT  (w p -- )  call digitalWrite(p, w)
* `AIN  (p -- w)`  call analogRead(p)
* `PWM  (w p -- )` call analogWrite(p, w)

#### examples
> 1 13 OUT ⏎
>> ok  i.e. digitalWrite(13, 1) called

### 32-bit Arithmatic (for Arduino Clock mostly)
* `D+   (d1 d0 -- d1+d0)` add two doubles
* `D-   (d1 d0 -- d1-d0)` subtract two doubles
* `DNG  (d0 -- -d0)` negate a double number

#### examples
> CLK DNG 1000 DLY CLK D+ ⏎

### System, Debug/Tracing
* `BYE  ( -- )` reset nanoFORTH
* `DMP  (a w -- )` dump nanoFORTH user dictionary from address 'a' for w bytes
* `TRC  (1|0 -- )` enable/diable execution tracing

## Opcode Formats
* `branching : 11BB aaaa aaaa aaaa            (12-bit absolute address)`
* `primitive : 10oo oooo                      (6-bit, i.e. 64 primitives)`
* `3-byte lit: 1011 1111 snnn nnnn nnnn nnnn  bf xxxx xxxx (16-bit signed integer)`
* `1-byte lit: 0nnn nnnn                      (0..127)`
* `n-byte str: len, byte, byte, ...           (used in print str)`

<br/>
<a href="page1.html">Review nanoFORTH command examples</a><br/>
<a href="page2.html">Intall nanoFORTH</a>





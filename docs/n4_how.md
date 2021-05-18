\page page3 How

## nanoFORTH Operations
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

## Opcode formats
>    `branching : 11BB aaaa aaaa aaaa            (12-bit absolute address)`<br>
>    `primitive : 10oo oooo                      (6-bit, i.e. 64 primitives)`<br>
>    `3-byte lit: 1011 1111 snnn nnnn nnnn nnnn  bf xxxx xxxx (16-bit signed integer)`<br>
>    `1-byte lit: 0nnn nnnn                      (0..127)`<br>
>    `n-byte str: len, byte, byte, ...           (used in print str)`<br>





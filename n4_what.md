\page page2 2 What
### Documentation

### NanoForth Opcode formats
>>    `branching : 11BB aaaa aaaa aaaa            (12-bit absolute address)`<br>
>>    `primitive : 10oo oooo                      (6-bit, i.e. 64 primitives)`<br>
>>    `3-byte lit: 1011 1111 snnn nnnn nnnn nnnn  bf xxxx xxxx (16-bit signed integer)`<br>
>>    `1-byte lit: 0nnn nnnn                      (0..127)`<br>
>>    `n-byte str: len, byte, byte, ...           (used in print str)`<br>

### Opcodes
#### Immediate (interactive mode)
>    :   VAR FGT DMP BYE

#### Branching (compile mode)
>    ;   IF  ELS THN BGN UTL WHL RPT FOR NXT I

#### Primitives
>    DRP DUP SWP OVR +   -   *   /   MOD NEG
>    AND OR  XOR NOT =   <   >   <=  >=  <>
>    @   !   C@  C!  .   ."  >R  R>  WRD HRE
>    CEL ALO SAV LD  TRC D+  D-  CLK DLY PIN
>    IN  OUT AIN PWM

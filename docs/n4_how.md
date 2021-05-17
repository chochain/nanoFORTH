\page page3 How

## Opcode formats
>    `branching : 11BB aaaa aaaa aaaa            (12-bit absolute address)`<br>
>    `primitive : 10oo oooo                      (6-bit, i.e. 64 primitives)`<br>
>    `3-byte lit: 1011 1111 snnn nnnn nnnn nnnn  bf xxxx xxxx (16-bit signed integer)`<br>
>    `1-byte lit: 0nnn nnnn                      (0..127)`<br>
>    `n-byte str: len, byte, byte, ...           (used in print str)`<br>

## Command Sets
### Immediate (interactive mode)
>    `:   VAR FGT DMP BYE`

### Branching (compile mode)
>    `;   IF  ELS THN BGN UTL WHL RPT FOR NXT I`

### Primitives (49 words)
>    `DRP DUP SWP OVR ROT +   -   *   /   MOD`<br>
>    `NEG AND OR  XOR NOT =   <   >   <=  >= `<br>
>    `<>  @   !   C@  C!  KEY EMT CR  .   ." `<br>
>    `>R  R>  WRD HRE CEL ALO SAV LD  TRC CLK`<br>
>    `D+  D-  DNG DLY PIN IN  OUT AIN PWM`




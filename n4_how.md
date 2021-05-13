\page page2 How

## Opcode formats
>    `branching : 11BB aaaa aaaa aaaa            (12-bit absolute address)`<br>
>    `primitive : 10oo oooo                      (6-bit, i.e. 64 primitives)`<br>
>    `3-byte lit: 1011 1111 snnn nnnn nnnn nnnn  bf xxxx xxxx (16-bit signed integer)`<br>
>    `1-byte lit: 0nnn nnnn                      (0..127)`<br>
>    `n-byte str: len, byte, byte, ...           (used in print str)`<br>

## Command Sets
### Immediate (interactive mode)
>     `:   VAR FGT DMP BYE`

### Branching (compile mode)
>     `;   IF  ELS THN BGN UTL WHL RPT FOR NXT I`

### Primitives
>     `DRP DUP SWP OVR +   -   *   /   MOD NEG`<br>
>     `AND OR  XOR NOT =   <   >   <=  >=  <> `<br>
>     `@   !   C@  C!  .   ."  >R  R>  WRD HRE`<br>
>     `CEL ALO SAV LD  TRC D+  D-  CLK DLY PIN`<br>
>     `IN  OUT AIN PWM`

## Tutorial

todo!


; Tiny FORTH
; T. NAKAGAWA
; 2004/08/02-05


.def	dicptr_L = r20
.def	dicptr_H = r21
.def	dicent_L = r22
.def	dicent_H = r23
.def	zero = r24

.equ	PFX_UDJ = 0x80
.equ	PFX_CDJ = 0xa0
.equ	PFX_CALL = 0xc0
.equ	PFX_PRIMITIVE = 0xe0
.equ	I_RET = 0xfe
.equ	I_LOOP = (PFX_PRIMITIVE | 25)
.equ	I_RDROP2 = (PFX_PRIMITIVE | 26)
.equ	I_I = (PFX_PRIMITIVE | 27)
.equ	I_P2R2 = (PFX_PRIMITIVE | 28)
.equ	STK_BGN = 0x0060
.equ	STK_END = 0x0060 + 48
.equ	DIC_BGN = 0x0060 + 48


.include "2313def.inc"


.cseg
.org	0


; Interruption Vector
	rjmp	main
	reti
	reti
	reti
	reti
	reti
	reti
	reti
	reti
	reti
	reti

main:
	clr	zero
	; Initialize the hardware stack
	ldi	r16, RAMEND
	out	SPL, r16
	; System init.
	rcall	initl

	; Initialize the stack and dictionary
	ldi	YL, LOW(STK_BGN)
	ldi	YH, HIGH(STK_BGN)
	ldi	XL, LOW(STK_END)
	ldi	XH, HIGH(STK_END)
	ldi	dicptr_L, LOW(DIC_BGN)
	ldi	dicptr_H, HIGH(DIC_BGN)
	ldi	dicent_L, LOW(0xffff)
	ldi	dicent_H, HIGH(0xffff)
	; Initialize the input buffer
	ldi	r16, ' '
	mov	r1, r16
	mov	r2, r16
	ldi	ZL, 0x03
	ldi	ZH, 0x00
main0:	st	Z+, zero
	cpi	ZL, 16
	brne	main0

	ldi	ZL, LOW(2 * msg_start)
	ldi	ZH, HIGH(2 * msg_start)
	rcall	putmsg

main1:	rcall	gettkn
	; keyword
	ldi	ZL, LOW(2 * key_runmode)
	ldi	ZH, HIGH(2 * key_runmode)
	rcall	find
	brne	main2
main11:	;; :
	cpi	r16, 0
	brne	main12
	rcall	compile
	rjmp	maina
main12:	;; VAR
	cpi	r16, 1
	brne	main13
	rcall	variable
	rjmp	maina
main13:	;; FGT
	cpi	r16, 2
	brne	main14
	rcall	forget
main14:	rjmp	maina
main2:	; User Defined Command
	rcall	lookup
	brne	main3
	adiw	ZL, 5
	rcall	execute
	rjmp	maina
main3:	; primitive
	ldi	ZL, LOW(2 * key_primitive)
	ldi	ZH, HIGH(2 * key_primitive)
	rcall	find
	brne	main4
	rcall	primitive
	rjmp	maina
main4:	; literal
	rcall	literal
	brne	main5
	st	-X, ZL
	st	-X, ZH
	rjmp	maina
main5:	;; error
	ldi	ZL, LOW(2 * msg_error)
	ldi	ZH, HIGH(2 * msg_error)
	rcall	putmsg
	rjmp	main1
maina:	ldi	r16, LOW(STK_END + 1)
	ldi	r17, HIGH(STK_END + 1)
	cp	XL, r16
	cpc	XH, r17
	brcs	mainb
	ldi	ZL, LOW(2 * msg_ovf)
	ldi	ZH, HIGH(2 * msg_ovf)
	rcall	putmsg
	ldi	XL, LOW(STK_END)
	ldi	XH, HIGH(STK_END)
	rjmp	main1
mainb:	ldi	ZL, LOW(2 * msg_ok)
	ldi	ZH, HIGH(2 * msg_ok)
	rcall	putmsg
	rjmp	main1

;
; Put a Message
;  IN: ZH:ZL=pointer to the string
;
putmsg:
	lpm
	tst	r0
	breq	putm1
	mov	r25, r0
	rcall	putchr
	adiw	ZL, 1
	rjmp	putmsg
putm1:	ldi	r25, 0x0d
	rcall	putchr
	ldi	r25, 0x0a
	rcall	putchr
	ret

;
; Get a Token
;  OUT: r1-r15=input string(terminated by ' ')
;
gettkn:
	ldi	r17, ' '
	ldi	ZH, 0x00
	; remove leading non-delimiters
	rcall	gettA
	cp	r1, r17
	brne	gettkn
	; remove leading delimiters
gett1:	rcall	gettA
gett2:	cp	r1, r17
	breq	gett1
	; return if tokens exist
	cp	r1, zero
	breq	gett3
	ret
gett3:	; read
	ldi	ZL, 0x01
gett4:	rcall	getchr
	;; RET
	cpi	r25, 0x0d	; '\r'
	brne	gett41
	ldi	r25, 0x0a	; '\n'
	rcall	putchr
	st	Z, r17
	rjmp	gett2
gett41:	;; BS
	cpi	r25, 0x08	; '\b'
	brne	gett42
	cpi	ZL, 1
	breq	gett4
	st	-Z, zero
	ldi	r25, ' '
	rcall	putchr
	ldi	r25, 0x08	; '\b'
	rcall	putchr
	rjmp	gett4
	;; CTRL
gett42:	cpi	r25, 0x20
	brcc	gett43
	rjmp	gett4
gett43:	;; INS
	cpi	ZL, 15
	breq	gett44
	st	Z+, r25
	rjmp	gett4
gett44:	;; Else
	ldi	r25, 0x08	; '\b'
	rcall	putchr
	ldi	r25, ' '
	rcall	putchr
	ldi	r25, 0x08	; '\b'
	rcall	putchr
	rjmp	gett4
gettA:	; shift left the buffer
	ldi	ZL, 0x01
gettA1:	cpi	ZL, 15
	breq	gettA2
	ldd	r16, Z+1
	st	Z+, r16
	rjmp	gettA1
gettA2:	st	Z, zero
	ret


;
; Process a Literal
;  IN: r1-r15=keyword
;  OUT: ZH:ZL=number, ZF=1/0(is literal/not literal)
;
literal:
	push	YL
	push	YH
	ldi	YL, 0x01
	ldi	YH, 0x00
	ldi	ZL, 0
	ldi	ZH, 0
	ld	r16, Y
lit1:	; Hex value
	cpi	r16, '$'
	brne	lit2
lit11:	adiw	YL, 1
	ld	r16, Y
	cpi	r16, ' '
	breq	lit4
	ldi	r16, LOW(16)	; *= 16
	ldi	r17, HIGH(16)
	rcall	mul16
	ld	r16, Y
	subi	r16, 'A'
	brcc	lit12
	subi	r16, -('A') + '0' + (10)
lit12:	subi	r16, -(10)
	add	ZL, r16
	adc	ZH, zero
	rjmp	lit11
lit2:	; Decimal value
	cpi	r16, '0'
	brcs	lit3
	cpi	r16, '9'+1
	brcc	lit3
lit21:	ld	r16, Y
	cpi	r16, ' '
	breq	lit4
	ldi	r16, LOW(10)	; *= 10
	ldi	r17, HIGH(10)
	rcall	mul16
	ld	r16, Y+
	subi	r16, '0'
	add	ZL, r16
	adc	ZH, zero
	rjmp	lit21
lit3:	clz
lit4:	pop	YH
	pop	YL
	ret


;
; Lookup the keyword from the Dictionary
; OUT: ZH:ZL=pointer to the entry, ZF=1/0(found/not found)
;
lookup:
	mov	ZL, dicent_L
	mov	ZH, dicent_H
lup1:	ldi	r16, LOW(0xffff)
	ldi	r17, HIGH(0xffff)
	cp	ZL, r16
	cpc	ZH, r17
	breq	lup2
	; read the keyword (r18, r19, r0)
	ldd	r18, Z+2
	ldd	r19, Z+3
	ldd	r0, Z+4
	cpi	r19, ' '
	breq	lup11
	cp	r0, r3
lup11:	cpc	r19, r2
	cpc	r18, r1
	breq	lup3	; found
	; not found
	ldd	r16, Z+0
	ldd	r17, Z+1
	mov	ZL, r16
	mov	ZH, r17
	rjmp	lup1
lup2:	clz
lup3:	ret


;
; Find the Keyword in a List
;  IN: r1-r15=keyword, ZH:ZL=pointer to the list
;  OUT: r16=id, ZF=1/0(found/not found)
;
find:
	ldi	r16, 0
	lpm
	adiw	ZL, 1
	mov	r17, r0
find1:	cp	r17, r16
	breq	find2
	; read the keyword (r18, r19, r0)
	lpm
	adiw	ZL, 1
	mov	r18, r0
	lpm
	adiw	ZL, 1
	mov	r19, r0
	lpm
	adiw	ZL, 1
	cpi	r19, ' '
	breq	find11
	cp	r0, r3
find11:	cpc	r19, r2
	cpc	r18, r1
	breq	find3
	inc	r16
	rjmp	find1
find2:	clz
find3:	ret


;
; Compile Mode
;
compile:
	push	XL
	push	XH
	mov	XL, dicptr_L
	mov	XH, dicptr_H
	rcall	gettkn
	; Write the header
	mov	ZL, dicent_L
	mov	ZH, dicent_H
	mov	dicent_L, XL
	mov	dicent_H, XH
	st	X+, ZL
	st	X+, ZH
	st	X+, r1
	st	X+, r2
	st	X+, r3
	mov	r16, r2
	cpi	r16, ' '
	brne	comp1
	st	-X, r16
	adiw	XL, 1
comp1:	; compile
	ldi	ZL, LOW(2 * msg_compile)
	ldi	ZH, HIGH(2 * msg_compile)
	rcall	putmsg
	rcall	gettkn
	; keyword
	ldi	ZL, LOW(2 * key_compilemode)
	ldi	ZH, HIGH(2 * key_compilemode)
	rcall	find
	breq	comp11
	rjmp	comp2
comp11:	;; ;
	cpi	r16, 0
	brne	comp12
	ldi	r16, I_RET
	st	X+, r16
	mov	dicptr_L, XL
	mov	dicptr_H, XH
	pop	XH
	pop	XL
	ret
comp12:	;; IF
	cpi	r16, 1
	brne	comp13
	st	Y+, XL
	st	Y+, XH
	ldi	r16, PFX_CDJ
	st	X+, r16
	adiw	XL, 1
	rjmp	comp1
comp13:	;; ELS
	cpi	r16, 2
	brne	comp14
	ld	ZH, -Y
	ld	ZL, -Y
	st	Y+, XL
	st	Y+, XH
	ldi	r16, PFX_UDJ
	st	X+, r16
	adiw	XL, 1
	mov	r18, XL
	mov	r19, XH
	sub	r18, ZL
	sbc	r19, ZH
	subi	r19, -0x10	; +4096
	ld	r16, Z
	or	r19, r16
	st	Z+, r19
	st	Z+, r18
	rjmp	comp1
comp14:	;; THN
	cpi	r16, 3
	brne	comp15
	ld	ZH, -Y
	ld	ZL, -Y
	mov	r18, XL
	mov	r19, XH
	sub	r18, ZL
	sbc	r19, ZH
	subi	r19, -0x10	; +4096
	ld	r16, Z
	or	r19, r16
	st	Z+, r19
	st	Z+, r18
	rjmp	comp1
comp15:	;; BGN
	cpi	r16, 4
	brne	comp16
	st	Y+, XL
	st	Y+, XH
	rjmp	comp1
comp16:	;; END
	cpi	r16, 5
	brne	comp17
	ld	ZH, -Y
	ld	ZL, -Y
	sub	ZL, XL
	sbc	ZH, XH
	subi	ZH, -0x10	; +4096
	ori	ZH, PFX_CDJ
	st	X+, ZH
	st	X+, ZL
	rjmp	comp1
comp17:	;; WHL
	cpi	r16, 6
	brne	comp18
	st	Y+, XL
	st	Y+, XH
	adiw	XL, 2
	rjmp	comp1
comp18:	;; RPT
	cpi	r16, 7
	brne	comp19
	ld	ZH, -Y
	ld	ZL, -Y
	adiw	XL, 2
	mov	r18, XL
	mov	r19, XH
	sbiw	XL, 2
	sub	r18, ZL
	sbc	r19, ZH
	subi	r19, -0x10	; +4096
	ori	r19, PFX_CDJ
	st	Z+, r19
	st	Z+, r18
	ld	ZH, -Y
	ld	ZL, -Y
	sub	ZL, XL
	sbc	ZH, XH
	subi	ZH, -0x10	; +4096
	ori	ZH, PFX_UDJ
	st	X+, ZH
	st	X+, ZL
	rjmp	comp1
comp19:	;; DO
	cpi	r16, 8
	brne	comp1a
	ldi	r16, I_P2R2
	st	X+, r16
	st	Y+, XL
	st	Y+, XH
	rjmp	comp1
comp1a:	;; LOP
	cpi	r16, 9
	brne	comp1b
	ldi	r16, I_LOOP
	st	X+, r16
	ld	ZH, -Y
	ld	ZL, -Y
	sub	ZL, XL
	sbc	ZH, XH
	subi	ZH, -0x10	; +4096
	ori	ZH, PFX_CDJ
	st	X+, ZH
	st	X+, ZL
	ldi	r16, I_RDROP2
	st	X+, r16
	rjmp	comp1
comp1b:	;; I
	cpi	r16, 10
	brne	comp1c
	ldi	r16, I_I
	st	X+, r16
comp1c:	rjmp	comp1
comp2:	; User Defined Command
	rcall	lookup
	brne	comp3
	adiw	ZL, 5
	sub	ZL, XL
	sbc	ZH, XH
	subi	ZH, -0x10	; +4096
	ori	ZH, PFX_CALL
	st	X+, ZH
	st	X+, ZL
	rjmp	comp1
comp3:	; keyword
	ldi	ZL, LOW(2 * key_primitive)
	ldi	ZH, HIGH(2 * key_primitive)
	rcall	find
	brne	comp4
	ori	r16, PFX_PRIMITIVE
	st	X+, r16
	rjmp	comp1
comp4:	; literal
	rcall	literal
	brne	comp5
	cpi	ZL, 128
	cpc	ZH, zero
	brcc	comp41
	st	X+, ZL
	rjmp	comp1
comp41:	ldi	r16, 0xff
	st	X+, r16
	st	X+, ZL
	st	X+, ZH
	rjmp	comp1
comp5:	; else
	ldi	ZL, LOW(2 * msg_error2)
	ldi	ZH, HIGH(2 * msg_error2)
	rcall	putmsg
	rjmp	comp1


;
; VARIABLE instruction
;
variable:
	push	XL
	push	XH
	mov	XL, dicptr_L
	mov	XH, dicptr_H

	; get the identifier
	rcall	gettkn

	; Write the header
	mov	ZL, dicent_L
	mov	ZH, dicent_H
	mov	dicent_L, XL
	mov	dicent_H, XH
	st	X+, ZL
	st	X+, ZH
	st	X+, r1
	st	X+, r2
	st	X+, r3
	mov	r16, r2
	cpi	r16, ' '
	brne	var1
	st	-X, r16
	adiw	XL, 1
var1:	mov	ZL, XL
	mov	ZH, XH
	adiw	ZL, 2
	cpi	ZL, 128
	cpc	ZH, zero
	brcc	var2
	st	X+, ZL
	rjmp	var3
var2:	adiw	ZL, 2
	ldi	r16, 0xff
	st	X+, r16
	st	X+, ZL
	st	X+, ZH
var3:	ldi	r16, I_RET
	st	X+, r16
	st	X+, zero	; data area
	st	X+, zero	; data area
	mov	dicptr_L, XL
	mov	dicptr_H, XH
	pop	XH
	pop	XL
	ret


;
; Forget Words in the Dictionary
forget:
	; get a word
	rcall	gettkn
	rcall	lookup
	breq	fgt1
	ldi	ZL, LOW(2 * msg_error3)
	ldi	ZH, HIGH(2 * msg_error3)
	rcall	putmsg
	ret
fgt1:	ldd	r16, Z+0
	ldd	r17, Z+1
	mov	dicent_L, r16
	mov	dicent_H, r17
	mov	dicptr_L, ZL
	mov	dicptr_H, ZH
	ret


;
; Virtual Code Execution
;  ZH:ZL=start address
;
execute:
	ldi	r16, 0xff
	st	Y+, r16
	st	Y+, r16

exec1:	ldi	r16, 0xff
	cp	ZL, r16
	cpc	ZH, r16
	brne	exec2
	ret
exec2:	ld	r16, Z+
exec3:	; literal(0-127)
	cpi	r16, 128
	brcc	exec4
	st	-X, r16
	st	-X, zero
	rjmp	exec1
exec4:	; literal(128-65535)
	cpi	r16, 0xff
	brne	exec5
	ld	r16, Z+
	ld	r17, Z+
	st	-X, r16
	st	-X, r17
	rjmp	exec1
exec5:	; RET: return
	cpi	r16, I_RET
	brne	exec6
	ld	ZH, -Y
	ld	ZL, -Y
	rjmp	exec1
exec6:	; UDJ: unconditional direct jump
	mov	r17, r16
	andi	r16, 0xe0
	andi	r17, 0x1f
	cpi	r16, PFX_UDJ
	brne	exec7
	ld	r16, Z
	sbiw	ZL, 1
	add	ZL, r16
	add	ZH, r17
	subi	ZH, 0x10	; -4096
	rjmp	exec1
exec7:	; CDJ: conditional direct jump
	cpi	r16, PFX_CDJ
	brne	exec8
	ld	r19, X+
	ld	r18, X+
	cp	r18, zero
	cpc	r19, zero
	brne	exec71
	ld	r16, Z
	sbiw	ZL, 1
	add	ZL, r16
	add	ZH, r17
	subi	ZH, 0x10	; -4096
	rjmp	exec1
exec71:	adiw	ZL, 1
	rjmp	exec1
exec8:	; CALL: subroutine call
	cpi	r16, PFX_CALL
	brne	exec9
	ld	r16, Z+
	st	Y+, ZL
	st	Y+, ZH
	sbiw	ZL, 2
	add	ZL, r16
	add	ZH, r17
	subi	ZH, 0x10	; -4096
	rjmp	exec1
exec9:	; primitive functions
	mov	r16, r17
	push	ZL
	push	ZH
	rcall	primitive
	pop	ZH
	pop	ZL
	rjmp	exec1


;
; Execute a Primitive Instruction
;  IN: r16=instruction
;
primitive:
prim1:	; DROP
	cpi	r16, 0
	brne	prim2
	adiw	XL, 2
	ret
prim2:	; DUP
	cpi	r16, 1
	brne	prim3
	ld	r17, X+
	ld	r16, X+
	st	-X, r16
	st	-X, r17
	st	-X, r16
	st	-X, r17
	ret
prim3:	; SWAP
	cpi	r16, 2
	brne	prim4
	ld	r19, X+
	ld	r18, X+
	ld	r17, X+
	ld	r16, X+
	st	-X, r18
	st	-X, r19
	st	-X, r16
	st	-X, r17
	ret
prim4:	; >R
	cpi	r16, 3
	brne	prim5
	ld	r17, X+
	ld	r16, X+
	st	Y+, r16
	st	Y+, r17
	ret
prim5:	; R>
	cpi	r16, 4
	brne	prim6
	ld	r17, -Y
	ld	r16, -Y
	st	-X, r16
	st	-X, r17
	ret
prim6:	; +
	cpi	r16, 5
	brne	prim7
	ld	r19, X+
	ld	r18, X+
	ld	r17, X+
	ld	r16, X+
	add	r16, r18
	adc	r17, r19
	st	-X, r16
	st	-X, r17
	ret
prim7:	; -
	cpi	r16, 6
	brne	prim8
	ld	r19, X+
	ld	r18, X+
	ld	r17, X+
	ld	r16, X+
	sub	r16, r18
	sbc	r17, r19
	st	-X, r16
	st	-X, r17
	ret
prim8:	; *
	cpi	r16, 7
	brne	prim9
	ld	r17, X+
	ld	r16, X+
	ld	ZH, X+
	ld	ZL, X+
	rcall	mul16
	st	-X, ZL
	st	-X, ZH
	ret
prim9:	; /
	cpi	r16, 8
	brne	prima
	ld	r17, X+
	ld	r16, X+
	ld	ZH, X+
	ld	ZL, X+
	rcall	div
	st	-X, ZL
	st	-X, ZH
	ret
prima:	; MOD
	cpi	r16, 9
	brne	primb
	ld	r17, X+
	ld	r16, X+
	ld	ZH, X+
	ld	ZL, X+
	rcall	div
	st	-X, r16
	st	-X, r17
	ret
primb:	; AND
	cpi	r16, 10
	brne	primc
	ld	r19, X+
	ld	r18, X+
	ld	r17, X+
	ld	r16, X+
	and	r16, r18
	and	r17, r19
	st	-X, r16
	st	-X, r17
	ret
primc:	; OR
	cpi	r16, 11
	brne	primd
	ld	r19, X+
	ld	r18, X+
	ld	r17, X+
	ld	r16, X+
	or	r16, r18
	or	r17, r19
	st	-X, r16
	st	-X, r17
	ret
primd:	; XOR
	cpi	r16, 12
	brne	prime
	ld	r19, X+
	ld	r18, X+
	ld	r17, X+
	ld	r16, X+
	eor	r16, r18
	eor	r17, r19
	st	-X, r16
	st	-X, r17
	ret
prime:	; =
	cpi	r16, 13
	brne	primf
	ld	r19, X+
	ld	r18, X+
	ld	r17, X+
	ld	r16, X+
	cp	r16, r18
	cpc	r17, r19
	breq	primTR
primFL:	ldi	r16, 0x00
	rjmp	prime1
primTR:	ldi	r16, 0x01
prime1:	ldi	r17, 0x00
	st	-X, r16
	st	-X, r17
	ret
primf:	; <
	cpi	r16, 14
	brne	primg
	ld	r19, X+
	ld	r18, X+
	ld	r17, X+
	ld	r16, X+
	cp	r16, r18
	cpc	r17, r19
	brcs	primTR
	rjmp	primFL
primg:	; >
	cpi	r16, 15
	brne	primh
	ld	r19, X+
	ld	r18, X+
	ld	r17, X+
	ld	r16, X+
	cp	r18, r16
	cpc	r19, r17
	brcs	primTR
	rjmp	primFL
primh:	; <=
	cpi	r16, 16
	brne	primi
	ld	r19, X+
	ld	r18, X+
	ld	r17, X+
	ld	r16, X+
	cp	r18, r16
	cpc	r19, r17
	brcc	primTR
	rjmp	primFL
primi:	; >=
	cpi	r16, 17
	brne	primj
	ld	r19, X+
	ld	r18, X+
	ld	r17, X+
	ld	r16, X+
	cp	r16, r18
	cpc	r17, r19
	brcc	primTR
	rjmp	primFL
primj:	; <>
	cpi	r16, 18
	brne	primk
	ld	r19, X+
	ld	r18, X+
	ld	r17, X+
	ld	r16, X+
	cp	r16, r18
	cpc	r17, r19
	brne	primTR
	rjmp	primFL
primk:	; NOT
	cpi	r16, 19
	brne	priml
	ld	r17, X+
	ld	r16, X+
	cp	r16, zero
	cpc	r17, zero
	breq	primTR
	rjmp	primFL
priml:	; @
	cpi	r16, 20
	brne	primm
	ld	ZH, X+
	ld	ZL, X+
	ldd	r16, Z+0
	ldd	r17, Z+1
	st	-X, r16
	st	-X, r17
	ret
primm:	; @@
	cpi	r16, 21
	brne	primn
	ld	ZH, X+
	ld	ZL, X+
	ldd	r16, Z+0
	st	-X, r16
	st	-X, zero
	ret
primn:	; !
	cpi	r16, 22
	brne	primo
	ld	ZH, X+
	ld	ZL, X+
	ld	r17, X+
	ld	r16, X+
	std	Z+0, r16
	std	Z+1, r17
	ret
primo:	; !!
	cpi	r16, 23
	brne	primp
	ld	ZH, X+
	ld	ZL, X+
	ld	r17, X+
	ld	r16, X+
	std	Z+0, r16
	ret
primp:	; .
	cpi	r16, 24
	brne	primq
	ld	ZH, X+
	ld	ZL, X+
	rcall	putnum
	ldi	r25, ' '
	rcall	putchr
	ret
primq:	; LOOP
	cpi	r16, 25
	brne	primr
	ld	r17, -Y
	ld	r16, -Y
	ld	ZH, -Y
	ld	ZL, -Y
	adiw	ZL, 1
	st	Y+, ZL
	st	Y+, ZH
	st	Y+, r16
	st	Y+, r17
	cp	ZL, r16
	cpc	ZH, r17
	brcc	primq1
	ldi	r16, 0x00
	rjmp	primq2
primq1:	ldi	r16, 0x01
primq2:	ldi	r17, 0x00
	st	-X, r16
	st	-X, r17
	ret
primr:	; RDROP2
	cpi	r16, 26
	brne	prims
	sbiw	YL, 4
	ret
prims:	; I
	cpi	r16, 27
	brne	primt
	ld	r19, -Y
	ld	r18, -Y
	ld	r17, -Y
	ld	r16, -Y
	st	Y+, r16
	st	Y+, r17
	st	Y+, r18
	st	Y+, r19
	st	-X, r16
	st	-X, r17
	ret
primt:	; P2R2
	cpi	r16, 28
	brne	primu
	ld	r17, X+
	ld	r16, X+
	st	Y+, r16
	st	Y+, r17
	ld	r17, X+
	ld	r16, X+
	st	Y+, r16
	st	Y+, r17
primu:	ret


;
; Put a Number
;  IN: ZH:ZL=number
;
putnum:
	ldi	r16, LOW(10)
	ldi	r17, HIGH(10)
	rcall	div
	cp	ZL, zero
	cpc	ZH, zero
	breq	putn1
	push	r16
	rcall	putnum
	pop	r16
putn1:	mov	r25, r16
	subi	r25, -('0')
	rcall	putchr
	ret


;------------------------------------------------------------------------------


; 16bit Multiplicatoin
;  IN: ZH:ZL=multiplicand, r17:r16=multiplier
;  OUT: ZH:ZL=product
mul16:
	push	YL
	push	YH
	clr	YL
	clr	YH
	ldi	r18, 16
mul1:	sbrs	ZL, 0
	rjmp	mul2
	add	YL, r16
	adc	YH, r17
mul2:	lsr	YH
	ror	YL
	ror	ZH
	ror	ZL
	dec	r18
	brne	mul1
	pop	YH
	pop	YL
	ret


; 16bit Division
;  IN: ZH:ZL=dividend, r17:r16=divisor
;  OUT: ZH:ZL=quotient, r17:r16=remainder
div:
	push	YL
	push	YH
	clr	YL
	clr	YH
	ldi	r18, 16
	lsl	ZL
	rol	ZH
	rol	YL
div1:	sub	YL, r16
	sbc	YH, r17
	brcc	div2
	add	YL, r16
	adc	YH, r17
	clc
	rjmp	div3
div2:	sec
div3:	rol	ZL
	rol	ZH
	rol	YL
	rol	YH
	dec	r18
	brne	div1
	lsr	YH
	ror	YL
	mov	r16, YL
	mov	r17, YH
	pop	YH
	pop	YL
	ret


;------------------------------------------------------------------------------


; Initialize
initl:
	ldi	r25, (EXP2(RXEN) | EXP2(TXEN))
	out	UCR, r25
	ldi	r25, 23	; 19200baud @ 7.3728MHz clock
	out	UBRR, r25
	ret


; Get a character
getchr:
	sbis	USR, RXC
	rjmp	getchr
	in	r25, UDR
	rjmp	putchr


; Put a character
putchr:
	sbis	USR, UDRE
	rjmp	putchr
	out	UDR, r25
	ret

key_runmode:
.db	3, ":  ", "VAR", "FGT"
key_compilemode:
.db	11, 0x3b, "  ", "IF ", "ELS", "THN", "BGN", "END", "WHL", "RPT", "DO ", "LOP", "I  "
key_primitive:
.db	25, "DRP", "DUP", "SWP", ">R ", "R> ", "+  ", "-  ", "*  ", "/  ", "MOD", "AND", "OR ", "XOR", "=  ", "<  ", ">  ", "<= ", ">= ", "<> ", "NOT", "@  ", "@@ ", "!  ", "!! ", ".  "
msg_start:
.db	"Tiny FORTH", 0x00, 0
msg_error:
.db	"?", 0x00
msg_ovf:
.db	"OVF", 0x00
msg_ok:
.db	"OK", 0x00, 0
msg_compile:
.db	">", 0x00
msg_error2:
.db	"!", 0x00
msg_error3:
.db	"??", 0x00, 0


; Register
;  r0: temporary
;  r1-15: input buffer (terminated by 0x00, separated by 0x20)
;  r16: temporary
;  r17: temporary
;  r18: temporary
;  r19: temporary
;  r20: DICPTR_L
;  r21: DICPTR_H
;  r22: DICENT_L
;  r23: DICENT_H
;  r24: zero
;  r25: system I/O
;  r26: XL (paramater stack L)
;  r27: XH (paramater stack H)
;  r28: YL (return stack L)
;  r29: YH (return stack H)
;  r30: ZL (temporary)
;  r31: ZH (temporary)
;
; Memory Map
;  0060-008f(48) : [return stack]-> <-[parameter stack]
;  0090-00df(80) : [dictionary]-> <-[hardware stack]

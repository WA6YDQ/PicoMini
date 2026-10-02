; progn.asm

conout	equ	0
conin	equ	0
CR		equ	13
LF		equ	10
NULL	equ	0

	; all programs start at TPA (0100h)
	org 100h	
	
	mvi	b	10		; counter
start	
	mvi a	CR
	out	conout
	mvi	a	LF
	out	conout
	mvi	a	'A'
	mvi c	26		; counter
	
loop
	out	conout
	inr a
	dcr	c
	jnz	loop
	mvi	a	CR
	out	conout
	mvi a	LF
	out	conout
	dcr	b
	jnz	start
	
	jmp	0
	

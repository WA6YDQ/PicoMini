; progn.asm

conout	equ	0
conin	equ	0
CR		equ	13
LF		equ	10
NULL	equ	0

	org 100h
	
	mvi a	CR
	out	conout
	mvi	a	LF
	out	conout
	mvi	a	'A'
loop
	out	conout
	inr a
	cpi	'G'
	jnz	loop
	mvi	a	CR
	out	conout
	mvi a	LF
	out	conout
	
	jmp	0
	

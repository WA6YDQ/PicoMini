; test1.asm
; 8080 test program

conout		equ		0h

	org	0h
start	mvi a	45h
loop	out		conout
		dcr	a
		cpi		64
		jnz		loop
l2		jmp		l2

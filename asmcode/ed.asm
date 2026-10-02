; ed.asm - simple text editor
; LINE - 80 char line buffer

; commands:
; a - append text to buffer
; p - list buffer
; q - quit ed
; s - save buffer to disk
; l - load a disk file to buffer
; n - new (clear buffer)
; x - exit ed with test for save
; h - display command list
; d - delete a line
; r - replace a line
; i - insert a line

conout		EQU	0
conin		EQU	0
CR			EQU	13
LF			EQU	10
ESC			EQU	1bh
NULL		EQU	0
WARMBOOT 	EQU 0
BOOT		EQU	0

; File Control Block 1 (5Ch thru 7Ch) FCB1 5C-7C (92d - 124d)
FCB				EQU	BOOT+5Ch
FCBDRIVECODE	EQU	FCB			; 1 byte for drive code (0-16) (0=default, 1=A, 2=B, 16=P)
FILENAME		EQU	FCB+1		; 8 char filename high bit 0 (default upper case)
FILETYPE		EQU	FCB+9		; 3 char file type in ascii, upper case
FCBEXTENT		EQU	FCB+12		; default 00h extent (EX) ranges from 0 to 31
FCBS1			EQU	FCB+13		; reserved by cp/m for bdos
FCBS2			EQU	FCB+14		; reserved for cp/m for bdos, typically set to 0
FCBRC			EQU	FCB+15		; record count, 0-128
FCBSTATUS		EQU	FCB+40		; returned status byte

BUFFERSIZE EQU 2000h	; 8192 bytes

	org	0100h
start
	mvi a	0			; mode 0=command, 1=append, 2=insert
	sta		MODE
	lxi	h	STARTMSG
	call	puts		; start up message
	
	lxi h	BUFFERSTART	; start by clearing the buffer
	lxi d	BUFFERSIZE
	call	clear_block
	
	lxi h	BUFFERSTART	; begin of text entry buffer
	shld	CURBUFFERPOS	; current position of the buffer
	
	
	
loop
	mvi a	0		; ed> prompt
	sta		MODE
	call	showprompt
	
	lxi h	LINE
	mvi c	80
	call	memset	; clear the line buffer
	lxi h	LINE
	mvi c	80
	call	gets	; read a line
	
	; point to LINE start, test commands
	lxi h	LINE
	mov a,m
	
	cpi	NULL
	jz	cronly			; user hit <cr> 
	
	cpi	'q'				; exit program
	jz	WARMBOOT		
	
	cpi	'a'				; append to buffer
	jz	append
	
	cpi	'p'				; print (list) buffer
	jz	list
	
	cpi	's'				; save buffer to disk
	jz	save
	
	cpi	'l'				; load file to buffer
	jz	load
	
	cpi	'h'				; show help
	jz	help
	
	cpi	'c'				; clear buffer
	jz	clear_buffer
	
	cpi	'd'				; delete a line
	jz	delete
	
	cpi 't'
	jz 	test
	
	; bad command?
	lxi h	ERRORMSG
	call	puts
	call	crlf
	jmp		loop


cronly		; user hit <cr>
	call	crlf
	jmp		loop





STARTMSG DB "Editor V1",CR,LF
		 DB	"Press 'h' for command list",CR,LF,NULL
		 
ERRORMSG DB CR,LF,"EH?",NULL


; ----------- COMMANDS --------------



; ----------------------------------------------------
help
	call	crlf
	lxi h	HELPMSG
	call	puts
	jmp		loop
	
HELPMSG	DB "commands:",CR,LF
		DB " a - append text to buffer",CR,LF
		DB " p - print buffer",CR,LF
		DB " q - quit ed",CR,LF
		DB " s - save buffer to disk",CR,LF
		DB " l - load a disk file to buffer",CR,LF
		DB " c - clear buffer",CR,LF
		DB " h - display this list",CR,LF
		DB " *x - exit ed with test for save",CR,LF
		DB " d - delete a line",CR,LF
		DB " *r - replace a line",CR,LF
		DB " *i - insert a line",CR,LF,NULL


; --------------------------------------------
test
	call	crlf
	mvi a	0
test1	
	push	psw
	call	printdecimal
	call	crlf
	pop		psw
	inr	a
	cpi		100
	jnz		test1
	jmp		loop


; ----------------------------------------------------
clear_buffer
	; fill the used buffer with NULL and resetting pointers
	lxi h	BUFFERSTART
	lxi d	BUFFERSIZE
	call	clear_block
	; reset pointers
	lxi h	BUFFERSTART
	shld	CURBUFFERPOS
	
	call	crlf
	jmp		loop
	
	
	; lhld	CURBUFFERPOS
	; xchg	; move to DE
	; lxi h	BUFFERSTART
	; lxi d	BUFFERSIZE
	
	;mov a,e		; low byte of BUFFERSIZE to A
	;sub l		; subtract low byte of BUFFERSTART
	;mov l,a		; save
	
	;mov a,d		high byte of BUFFERSIZE to A
	;sbb h		; subtract high byte w/borrow
	;mov h,a		; save in A
	;xchg		; difference (counter) in DE
	
	;lxi h	BUFFERSTART

	

; -----------------------
append		; append text to buffer

	call	crlf
	mvi a	1
	sta		MODE
	call	showprompt		; show append prompt
	
	lxi h	LINE
	mvi c	80
	call	memset				; clear line
	lxi h	LINE
	mvi c	80
	call	gets				; get a line of text
	
	; test for ESC to end
	cpi	ESC
	jz	append_end
	
	; test for command
	lxi h	LINE
	mov a,m
	cpi '.'
	jnz		append_saveline
	
	; got command marker '.'
	inx h		; point to actual command
	mov a,m
	cpi	'x'		; exit append routine?
	jz		append_end
	
	; not a command - 
	dcx	h		; point back to '.'	
	
append_saveline		; save LINE to buffer
	lhld	CURBUFFERPOS		; get current buffer position
	lxi d	LINE
	
append_save0
	ldax d						; get char from LINE
	cpi		NULL
	jnz		append_save1
	
	; end of line - save CR/LF at EOL then get another line
	mvi a	CR
	mov	m,a						; end of line marker
	inx h
	mvi a	LF
	mov m,a
	inx h
	shld	CURBUFFERPOS
	jmp		append				; get another line
	
append_save1	; save LINE to text buffer
	mov	m,a		; save char
	inx h
	inx d
	jmp		append_save0
	
		
append_end
	; return to main
	mvi a	0		; set mode to command
	sta		MODE
	call	crlf
	jmp		loop
	
	
; ----------- delete --------------------
delete		; delete a single line

; HL points to 'd'. skip ahead and get the line number to delete
; UD2B converts string pointed to by DE to number in HL


	
; ----------------------------------------
list		; display buffer

	call	crlf
	
	;; show line number
	lxi h	1		; line number
	shld	LINENUMBER
	call	UB2D		; convert binary in HL to decimal string pointed to by DE, null term
	xchg				;; de -> hl
	call	puts		; show ln
	mvi a	':'
	call	putc
	call	putc		; two :: between line number and text
	
	lxi	h	BUFFERSTART
	
list1
	mov	a,m			; get char from buffer
	cpi	NULL
	jz	list_end
	
	cpi	CR
	jnz	list2
	
	; got CRLF - show, then print a new line number
	call	crlf
	inx h	; skip CR
	inx h	; skip LF
	mov a,m
	cpi	NULL
	jz	list_end
	
	; not null - print line#
	push h		;; save address
	lhld	LINENUMBER
	inx h
	shld	LINENUMBER
	call	UB2D
	xchg	;; de -> hl
	call	puts
	pop	h
	mvi a	':'
	call	putc
	call	putc

list2
	; now show char
	mov	a,m
	call	putc
	inx	h
	jmp	list1
	
list_end
	; return to main
	call	crlf
	jmp		loop


; ---------------------------------------
save		; save buffer to disk

	call	crlf
	lxi h	SAVEMSG1
	call	puts
	
	lxi	h	LINE
	mvi c	80
	call	memset
	lxi	h	LINE
	mvi c	80
	call	gets		; get a filename
	
	; save the filename to FCB
	lxi d	FILENAME
	lxi h	LINE
	
save1	; save the filename up to '.'
	mov a,m		; get char from LINE
	cpi	'.'
	jz	save2
	stax d
	inx h
	inx d
	jmp	save1
	
save2	; skip '.'
	inx h		; skip '.'
	lxi d		FILETYPE
	
save3	; save the filetype
	mov a,m
	cpi	NULL
	jz		save4
	stax d	
	inx h
	inx d
	jmp		save3
	
save4	; point DE to start, HL to end and save buffer
	lxi d	BUFFERSTART
	lhld	CURBUFFERPOS
	out 248
	
	; get status byte
	lda		FCBSTATUS
	cpi		0
	jnz		save_fail
	call	crlf
	lxi h	SAVEMSGOK
	call	puts
	jmp		loop
	
save_fail
	call	crlf
	lxi h	SAVEMSGFAIL
	call	puts
	jmp		loop
	
	
	
	
SAVEMSG1	DB	"Filename: ",NULL	
SAVEMSGOK	DB	"File Saved",CR,LF,NULL
SAVEMSGFAIL DB  "Save Error",CR,LF,NULL	



; -----------------------------------
load		; load file into buffer

	call	crlf
	lxi h	LOADMSG1
	call	puts
	
	lxi	h	LINE
	mvi c	80
	call	memset
	lxi	h	LINE
	mvi c	80
	call	gets		; get a filename
	
	; save the filename to FCB
	lxi d	FILENAME
	lxi h	LINE
	
load1	; save the filename up to '.'
	mov a,m		; get char from LINE
	cpi	'.'
	jz	load2
	stax d
	inx h
	inx d
	jmp	load1
	
load2	; skip '.'
	inx h		; skip '.'
	lxi d		FILETYPE
	
load3	; save the filetype
	mov a,m
	cpi	NULL
	jz		load4
	stax d	
	inx h
	inx d
	jmp		load3
	
load4	; point HL to memory start for loading
	lxi h	BUFFERSTART
	in	248		; call load routine - if return code is 0
				; then HL points to last char loaded
	
	; get status byte
	lda		FCBSTATUS
	cpi		0
	jnz		load_error
	
	shld	CURBUFFERPOS		; save HL address of last byte loaded
	
	; show load success
	call	crlf
	lxi h	LOADMSGOK
	call	puts
	jmp		loop			; return to command prompt
	
load_error		; show load error
	call	crlf
	lxi h	LOADMSGFAIL
	call	puts
	jmp		loop
	
LOADMSGFAIL	DB	"File Load Failed",CR,LF,NULL
LOADMSGOK	DB	"File Loaded",CR,LF,NULL
LOADMSG1	DB	"Filename: ",NULL




; --------- SUBROUTINES -----------
; ------------------------------------------
showprompt	; display prompt depending on mode in B
	push	psw
	push	h
	lda		MODE	; get current mode
	cpi	0
	jnz sp1
	; show cmd prompt
	lxi h	CMDPROMPT
	call	puts
	pop h
	pop psw
	ret
sp1	
	cpi 1
	jnz	sp2
	; show append prompt
	lxi h	APPENDPROMPT
	call	puts
	pop h
	pop psw
	ret
sp2
	cpi	2
	jnz	sp3
	; show insert prompt
	lxi h	INSERTPROMPT
	call	puts
	pop h
	pop psw
	ret
sp3
	pop h
	pop psw
	ret
	
CMDPROMPT DB "ed>",NULL
APPENDPROMPT DB "a>",NULL
INSERTPROMPT DB "i>",NULL


; ---------- move block --------------
; move a block of memory. 
; HL points to start of FROM address
; DE points to start of TO address
; BC is the count (size) 
; A = 0, move DOWN. A = 1, move UP
move_block

	cpi	0
	jz		move_down
	
move_up
	
	
move_down		; counter = last_memory_used - start_of_TO_address
	mov a,m
	stax d
	inx	d
	inx h
	dcx b	; dec counter
	mov a,b
	ora c
	jnz	move_down
	ret
	
	

	
	
	
	

; ---------- clear block -----------
				; clear a block of memory
clear_block		; HL holds start address, DE holds size of block

	
	mvi a	NULL
	
clear_block1
	mov m,a
	inx h
	dcr e
	jnz	clear_block1
	
	mvi	a	0	; if D is already 0, we're done
	cmp	d
	jz	clear_block2
	
	mvi a	NULL
	dcr	d
	jnz		clear_block1
	
clear_block2
	ret
	
	

; -------------------------------
crlf	; send a CR LF to console
		push	psw
		mvi		a	CR
		call	putc
		mvi		a	LF
		call	putc
		pop		psw
		ret

; -------------------------------------------
putc
	; output a character in A to console
	out		conout
	ret
	
	
; ------------------------------------------	
puts
	; output a null terminated string
	; addressed by HL
	mov a,m
	cpi		NULL
	rz
	cpi		'$'		; for cp/m compatability
	rz
	call putc
	inx h
	jmp	puts
	

; -----------------------------------------
memset
	; set a block of memory to NULL
	; HL points to block, c is the counter
	mvi a,NULL
	mov m,a
	dcr c
	cmp c
	rz
	inx h
	jmp memset
	

; ------------------------------------
getc	- blocking input: get/return a single char

	in	conin
	cpi	feh
	rnz
	jmp	getc	
	
; ------------------------------------------------------------	
gets
	; blocking character input - get a string terminated by \r into the buffer
	; read max (c) chars from console (conin)
	; chars are stored on (HL) (HL preset by calling routine)
	; c is a countdown counter (preset by calling routine to 80)
	; termination is \r
	; last char in buffer is NULL (\r is NOT saved)

	
	call	getc	; get a char
	
	; test backspace
	cpi		08h		; backspace
	jnz		gets1
	
	; got BS
	mvi a	80		;; test if we are at buffer start
	cmp	c
	jz		gets	; we are - do nothing
	
	; delete last char
	dcx h			; point back 1 in buffer
	mvi	a,NULL		; save NULL in buffer
	mov	m,a
	inr	c			; increment counter
	; clear char on display
	mvi a	08h		;; show backspace
	call	putc
	mvi a	' '		;; blank the char under cursor
	call	putc
	mvi a	08h
	call	putc
	jmp		gets

gets1
	; test ctrl-U (delete line)
	cpi 15h		; ^U
	jnz	gets2
	
	lxi h	LINE		; restore pointers
	mvi c	80
	call	memset
	lxi h	LINE
	mvi c	80
	mvi a	'^'
	call	putc
	mvi a	'U'
	call	putc
	call	crlf
	mvi a	'>'
	call	putc
	jmp		gets

gets2
	; test ctrl-C (warm boot)
	cpi	03h
	jnz	gets_save
	; clear LINE 
	lxi h	LINE
	mvi c	80
	call 	memset
	mvi a	'^'
	call	putc
	mvi a	'C'
	call	putc
	jmp	WARMBOOT

gets_save	
	cpi	ESC			; don't echo, test for ESC
	rz				; return with ESC in A
	
	call	putc	; echo char
	cpi	CR
	rz
	
	; save to LINE
	mov m,a
	inx h
	dcr c
	mov a,c
	cpi 0
	jnz	gets
	;; when counter = 0, ignore last char, wait for \r
	inr c
	dcx h
	jmp		gets
	

/ ---------------------------------------------------------------------------------
; subroutine divide - divide number in HL by B. Dividend in HL, divisor in B
divide
		mvi 	c 0x08		; counter
up		
		dad 	h			; calling routine supplies HL and B
		mov 	a,h
		sub 	b
		jc 		down
		mov 	h,a
		inr 	l
down		
		dcr 	c
		jnz 	up			; ie: 59D / 10D L=5, H=9
		ret					; result in HL (remainder/quotient)



; subroutine printdecimal - convert value in A to decimal, print it to CON
; NOTE: range in A is 0-99 only. 
printdecimal
		push psw
		push b
		push d
		push h
		
		mov 	l,a			; put value in L
		mvi 	h	0x00	; MSB is 0
		mvi 	b	0x0a	; dividing by 10D
		call	divide
		mov 	a,l			; get quotient
		adi 	0x30		; to ascii
		call	putc
		mov 	a,h			; get remainder
		adi 	0x30		; to ascii
		call	putc
		
		pop h
		pop d
		pop b
		pop psw
		ret
; -------------------------------------------------------------------------------		
		
ERROF	; overflow error
		lxi h	OVFLOW
		call	puts
		ret
		
OVFLOW	DB	"Overflow Error in math routine",CR,LF,0

;----------------------------------------------------------------------------;		
; These next routines come from                                              ;
; https://github.com/lvisser-code/8080-8085_Math_Library/blob/main/UIML.asm  ;
;----------------------------------------------------------------------------;


;------------------------------- UD2B ---------------------------------
; Convert decimal number to binary
; On call: DE = ptr to decimal number in ASCII
; On retn: HL = binary result, DE is advanced past decimal number.
;----------------------------------------------------------------------
UD2B:  
		LDAX D          ;Skip any leading blanks
        INX D
        CPI ' '
        JZ UD2B
        DCX D
        LXI H,0         ;Initialize result = 0
UD2B1:  LDAX D          ;Fetch next ascii character
        SUI 48          ;Convert char to BCD digit if possible
        MOV C,A         ;Save (character-48) in C
        RM              ;Is it a digit 0 thru 9...
        CPI 10
        RP              ;...If not, then exit
        PUSH D          ;...If so, save buffer pointer
        LXI D,10        ;...Multiply partial result by 10...
        CALL UMUL       ;...(also) checking for overflow...
        MVI D,0         ;...and add in value of digit...
        MOV E,C
        CALL UADD       ;HL = HL*10 + digit
        POP D           ;Recall buffer pointer
        INX D           ;Bump buffer pointer
        JMP UD2B1       ;...and we're ready for next character
        
        
        
;------------------------------- UB2D ---------------------------------
; Convert binary number to ASCII decimal
; On call: HL = binary number, DE = ptr to string location
; On retn: string 0 terminated, A = number of characters.
;----------------------------------------------------------------------
UB2D:   
		PUSH B
        PUSH D
        PUSH H
        MVI C,0         ;C=digit counter
        PUSH D          ;Save a copy of ptr
UB2D1:  PUSH D          ;Save DE ptr prior to divide
        LXI D,10
        CALL UDIV       ;HL = HL / 10
        MOV A,E         ;E = remainder
        ADI 48          ;Convert to ASCII char
        POP D           ;Recall DE ptr
        STAX D          ;Move char to string
        INX D           ;Inc ptr
        INR C           ;Inc counter
        MOV A,H         ;If HL > 0...
        ORA L
        JNZ UB2D1       ;...loop
        SUB A
        STAX D          ;Move 0 to end of string
        INR C           ;Inc digit count
;Reverse string
        DCX D           ;Ptr to end of string
        POP H           ;Ptr to beginning of string
        PUSH B          ;Save a copy of digit count
        DCR C
UB2D2:  MOV B,M         ;Leading char to temp
        LDAX D          ;Get next trailing char
        MOV M,A         ;...move it
        MOV A,B         ;Get leading char
        STAX D          ;...move it
        INX H           ;Update pointers
        DCX D
        MOV A,C         ;Decrement count by 2
        SUI 2
        JZ UB2D3        ;Check whether done
        JC UB2D3
        MOV C,A
        JMP UB2D2       ;Loop
UB2D3:  POP B           ;Recall digit count
        MOV A,C         ;A = count
        POP H
        POP D
        POP B
        RET
        
        
        
;------------------------------- UMUL ---------------------------------
; Integer multiplication.
; On call: Registers HL and DE contain the multiplicands.
; On retn: HL contains the result.
;----------------------------------------------------------------------
UMUL:   PUSH D
        XRA A           ;Test for HL less than 256
        ADD H
        JZ UMUL1        ;Branch if HL less than
        XRA A
        ADD D           ;Else, DE must be < 256...
        CNZ ERROF       ;...or overflow would result
        XCHG            ;HL now has an op < 256
UMUL1:  MOV A,L         ;Move 255 or less multiplier to A
        LXI H,0         ;Initialize partial product
UMUL2:  STC
        CMC
        RAR             ;Rotate multiplier right off end
        JNC UMUL3       ;If bit shifted out was 0, skip
        DAD D           ;Else, add multiplicand to partial product
        CC  ERROF       ;...while checking for overflow
UMUL3:  XCHG
        DAD H           ;Shift multiplicand left 1 bit...
        XCHG
        ORA A
        JNZ UMUL2       ;Branch to top of loop if mult is non-0
        POP D
        RET


;------------------------------- UDIV ---------------------------------
; Integer division.
; On call: HL = dividend, DE = divisor.
; On retn: HL = quotient, DE = remainder.
;----------------------------------------------------------------------
UDIV:   PUSH B
        MOV A,D         ;If divisor MSB = 0...
        ORA A
        JNZ UDIV0       ;...then skip
;Check for special cases
        MOV A,E         ;If divisor LSB = 0...
        ORA A
        CZ  ERROF       ;...then call error handler (divide by 0)
        CPI 1           ;If divisor = 1...
        JZ UDIVF1       ;...then branch to fast divide by 1
        CPI 2           ;If divisor = 2...
        JZ UDIVF2       ;...then branch to fast divide by 2

UDIV0:  MOV C,L         ;Move dividend (=rem) to BC
        MOV B,H
        LXI H,0         ;initialize quotient = 0...
        PUSH H          ;...on top of stack (TOS)
        INR L           ;Initialize HL (pos) = 1
;Now BC = rem, DE=div, HL = pos, TOS=quo
;Shift pos & div left until rem >= div
UDIV1:  MOV A,D         ;If msb of div = 1...
        RAL
        JC UDIV3        ;...jump
        DAD H           ;pos = pos*2 (shift left)
        XCHG 
        DAD H           ;div = div*2 (shift left)
        XCHG
        MOV A,C         ;If div < rem...
        SUB L
        MOV A,B
        SBB H
        JNC UDIV1       ;...loop

UDIV2:  CALL UDIVR      ;pos = pos/2 (shift right)
        JZ UDIV4        ;If pos = 0, we're done
        XCHG            ;Now HL=div
        CALL UDIVR      ;div = div/2 (shift right)
        XCHG            ;Now HL=pos
UDIV3:  MOV A,C         ;If div > rem...
        SUB E
        MOV A,B
        SBB D
        JC UDIV2        ;...loop
        MOV A,C         ;rem = rem - div...
        SUB E
        MOV C,A
        MOV A,B
        SBB D
        MOV B,A
        XCHG            ;Now HL=div
        XTHL            ;Now BC=rem, DE=pos, HL=quo, TOS=div
        DAD D           ;quo = quo + pos
        XTHL            ;Now BC=rem, DE=pos, HL=div, TOS=quo
        XCHG            ;Now HL=pos
        JMP UDIV2       ;Loop

UDIV4:  POP H           ;Get quotient to HL
        MOV E,C         ;Move final rem to DE
        MOV D,B
        POP B
        RET

;Fast divide by 1
UDIVF1: LXI D,0         ;Remainder = 0
        POP B
        RET

;Fast divide by 2
UDIVF2: XRA A           ;Clear CY
        MOV A,H         ;Shift H
        RAR
        MOV H,A
        MOV A,L         ;Shift L
        RAR
        MOV L,A
        LXI D,0         ;Set DE remainder = CY
        MOV A,E
        RAL
        MOV E,A
        POP B
        RET


;Right shift HL, remainder returned in CY, Z set if zero
UDIVR:  XRA A           ;Clear CY
        MOV A,H         ;Shift H
        RAR
        MOV H,A
        MOV A,L         ;Shift L
        RAR
        MOV L,A
        ORA H           ;Set zero flag if both H and L = 0
        RET







;------------------------------- UADD ---------------------------------
; Integer addition.
; On call: Registers DE and HL contain the addends.
; On retn: HL contains the result.
;----------------------------------------------------------------------
UADD:   DAD D           ;HL=HL+DE: Only CY flag affected
        CC  ERROF       ;If overflow, then call error handler
        RET


;------------------------------- USUB ---------------------------------
; Integer subtraction.
; On call: Register HL contains minuend and DE contains subtrahend.
; On retn: HL contains the result.
;----------------------------------------------------------------------
USUB:   MOV A,L         ;Get low byte of minuend
        SUB E           ;Subtract low byte of subtrahend
        MOV L,A         ;L = result
        MOV A,H         ;Get high byte of minuend
        SBB D           ;Subtract high byte of subtrahend
        MOV H,A         ;H = result
        CC ERROF        ;If underflow, then call error handler
        RET





; ---------------------------------------------
; ---------------------------------------------

; holds current mode for prompt
MODE DS 1

; holds current buffer position
CURBUFFERPOS DS 2	

; holds 4 digit hex line number (print buffer)
LINENUMBER DS 2

; 80 char line buffer
LINE DS 80

; text buffer
	; org	BUFFERSTART
BUFFERSTART DS 8192
	

	

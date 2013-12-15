;----------------------------------------------------------------------------
; generally useful stuff
CR		equ	0Dh	; carriage return
LF		equ	0Ah	; line feed
ESCAPE_KEY	equ	1Bh	; ASCII code of Esc key

puts	macro	s
; put a string to the console
	push	dx
	push	ax
	mov	dx,OFFSET s 
	mov	ah,9
	int 	21h
	pop	ax
	pop	dx
endm


putchar	macro	c
; put a single character to the console
	push	dx
	push	ax
	mov	dl,c
	mov	ah,2
	int	21h
	pop	ax
	pop	dx
endm

string macro label,s
; add carriage return, line feed and money sign to string with a label
; so that DOS string output stuff is easier to type...
label 	db	s,CR,LF,'$'
endm

cstring macro label,s
; add null at end of a string
label 	db	s,0
endm


push_for_c	macro
; save all registers C considers sacred
	push	ebp
	mov	ebp,esp
	push	ebx
	push	esi
	push	edi
endm

pop_for_c	macro
; restore all C holy registers
	pop	edi
	pop	esi
	pop	ebx
	pop	ebp
endm

cparameter	equ	[ebp+8]		; first C parameter
PHAR_SEG	equ	014h		; Data segment for Pharlap programs
REAL_SEG 	equ 	034h		; Segment for lower 1 meg of memory


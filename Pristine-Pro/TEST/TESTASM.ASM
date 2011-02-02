
;----------------------------------------------------------------------------
; generally useful stuff
; :ts=8	(tell Z to set tabs to 8)
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

cparameter	equ	[ebp+8]		;first C parameter

cgroup	group	code
dgroup	group	data

data 	segment dword 'DATA'
PUBLIC counter
	counter dd 0000H 

data ends

code	segment dword 'CODE'

	assume cs:cgroup
	assume ds:dgroup

public drive_info
drive_info proc near
	push_for_c
	mov ebx,cparameter
	inc ebx		; 0 means current drive, 1 is A:,  2 is B: ...
	mov ah,44h
	mov al,08h
	int 21h
	jc bad
	or eax,100h
	jmp done
bad:	mov eax,0
done: 	pop_for_c
	ret
drive_info endp


code	ends

	end

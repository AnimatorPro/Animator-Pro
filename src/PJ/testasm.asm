REAL_SEG 	equ 	034h		; Segment for lower 1 meg of memory

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

	public real_peek_word
real_peek_word proc near
	push ebp
	mov ebp,esp
	push es

	mov ax,REAL_SEG	;Get real segment
	mov es,ax		;into ES register
	mov edx,[ebp+8] ;Fetch parameter
	xor eax,eax		;clear hi bits
	mov ax,es:[edx]	;Look up word

	pop es
	pop ebp
	ret
real_peek_word endp

dos_put_char proc near
	push ebp
	mov ebp,esp
	pushad

	mov edx,[ebp+8]
	mov ah,02H
	int 21H

	popad
	pop ebp
	ret
dos_put_char endp

code	ends
	end

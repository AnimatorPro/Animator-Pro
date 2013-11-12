;skip.asm - low level stuff to compress pictures into FLI's.

;:ts=10

	TITLE   skip
_TEXT	SEGMENT  BYTE PUBLIC 'CODE'
_TEXT	ENDS
_DATA	SEGMENT  WORD PUBLIC 'DATA'
_DATA	ENDS
CONST	SEGMENT  WORD PUBLIC 'CONST'
CONST	ENDS
_BSS	SEGMENT  WORD PUBLIC 'BSS'
_BSS	ENDS
DGROUP	GROUP	CONST,	_BSS,	_DATA
	ASSUME  CS: _TEXT, DS: DGROUP, SS: DGROUP, ES: DGROUP
_TEXT      SEGMENT

	PUBLIC	_tnsame
;tnsame(s2x,wcount,mustmatch)
s2x	equ [bp+4+2]
wcount	equ word ptr[bp+8+2]
mumatch	equ [bp+10+2]
_tnsame	PROC far
	push bp
	mov bp,sp
	push ds
	push si
	push di
	push bx

				
	les	di,s2x		;get starting address in es:di
	mov	dx,wcount		;dx is 'dif_count' return value
	mov	bx,dx		;bx is # of pixels left
	mov	si,0		;si is # of pixels examined
alp:
				;break out of loop if less than 4 pixels
				;left to examine
	cmp	bx,mumatch
	js	zalp

	;same_count = bsame(s2x,wcount)
	mov cx,bx
	mov al,es:[di]
	rep scasb
	inc cx
	sub di,2
	mov ax,bx
	sub ax,cx

	cmp ax,mumatch			;if mustmatch or more
	jns gotsame		;go truncate dif_count
	add	si,ax
	add	di,ax
	sub	bx,ax
	jmp	alp
gotsame:
	mov	dx,si		
zalp:
	mov ax,dx		;return dif_count
	pop bx
	pop di
	pop si
	pop ds
	pop bp
	ret

_tnsame	ENDP


_TEXT	ENDS
END

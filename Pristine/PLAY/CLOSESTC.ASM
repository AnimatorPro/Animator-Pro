
;:ts=10

;This module computes the rgb average of a pixel and it's 9 neighbors...

	TITLE   closestc
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


;closestc(rgb,cmap,count)
rgb	equ	dword ptr[bp+4+2]
cmap	equ	dword ptr[bp+8+2]
count	equ	word ptr[bp+12+2]
closest	equ	[bp-2]
	PUBLIC _closestc
_closestc	proc far
	push bp
	mov bp,sp
	sub  sp,2
	push ds
	push si
	push di
	push bx

	;r = bl, g = bh, b = ch
	lds si,rgb
	mov bx,[si]
	mov ch,[si+2]
	;cmap in ds:si
	lds si,cmap
	;di is the 'closest difference'
	mov di,32000	;bigger than could get really
	xor cl,cl


onecolor:
	;calculate difference in colors into dx
	lodsb	;fetch red
	sub	al,bl
	imul	al
	mov	dx,ax
	lodsb	;fetch green
	sub	al,bh
	imul	al
	add	dx,ax
	lodsb	;fetch blue
	sub	al,ch
	imul	al
	add	dx,ax

	;see if it's a better match than we've got so far
	cmp	di,dx
	jle	nonewdif

	;aha, here we've got a closer color
	mov	closest,cl
	mov	di,dx

nonewdif:
	inc	cl
	dec	count
	jnz	onecolor

	mov	al,closest	;return closest...
	xor	ah,ah		;make it an int for C

	pop bx
	pop di
	pop si
	pop ds
	mov sp,bp
	pop bp
	ret
_closestc	endp

_TEXT	ENDS
END

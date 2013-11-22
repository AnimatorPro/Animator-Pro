;un5brun.asm - low level routines to uncompress first frame of fli into
;1/5 size screen (by uncompressing 1 out of 5 lines, and skipping  past
;the other 4).


	TITLE   un5brun

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
	PUBLIC _un5brun
	;_un5brun(cbuf, screen, linect)
_un5brun PROC far
linect equ word ptr[bp+12+2]
	;save the world and set the basepage
	push bp
	mov bp,sp
	sub sp,4
	push es
	push ds
	push si
	push di
	push bx
	push cx

	cld	;clear direction flag in case Aztec or someone mucks with it.

	lds si,[bp+4+2]
	les di,[bp+8+2]
	mov	dx,di	;keep pointer to start of line in dx
	xor	ah,ah	;clear hi bit of ah cause lots of unsigned uses to follow
linelp:
	mov	di,dx
	lodsb		;get op count for this line
	mov bl,al  
ulcloop:
	lodsb	; load op/count
	test al,al
	js ucopy
	mov cx,ax ;move count to cx
	lodsb	  ;value to repeat in al
	rep stosb ;store same value again and again...
	dec bl	  ;through with this line yet????
	jnz ulcloop
	jmp ulcout
ucopy:
	neg al
	mov cx,ax ;get sign  correctec copy count
	rep movsb 
	dec bl
endulcloop:
	jnz ulcloop
ulcout:			;advance to next line...
	add	dx,320		;dest 1 line forwards
	mov bx,4		;decompressed 1 line, now skip 4
sk4:
	lodsb
	mov cx,ax	;count down ops in this line
skloop:
	lodsb		;get op/count byte
	test al,al
	js	scopy	;if signed its variable length
	inc si		;else just followed by one byte
	loop skloop
	jmp endskloop
scopy:
	neg al
	add si,ax
	loop skloop
endskloop:
	dec bx
	jnz sk4

	dec linect
	jnz	linelp

	pop cx
	pop bx
	pop di
	pop si
	pop ds
	pop es
	mov	sp,bp
	pop	bp
	ret	

_un5brun ENDP

	public _un5copy
_un5copy proc far
			;un5copy(chunk+1,p,BPR,f->h>>2);
source	equ	[bp+6]
dest	equ [bp+10]
bpr		equ [bp+14]
height	equ [bp+16]
	push bp
	mov bp,sp
	push si
	push di
	push ds
	push es
	push bx

	lds	si,source
	les	di,dest
	mov	dx,bpr
	mov bx,height
	mov ax,dx
	add ax,ax
	add	ax,ax	;ax = 4x bpr = what to add at end of line...
	shr dx,1	;doing word moves...
	jmp	zline4
line4:
	mov cx,dx
	rep movsw
	add	si,ax
zline4:
	dec	bx
	jns	line4


	pop bx
	pop es
	pop ds
	pop di
	pop si
	pop bp
	ret
_un5copy endp

_TEXT	ENDS
END

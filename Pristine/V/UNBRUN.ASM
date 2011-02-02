;unbrun.asm - low level routine to uncompress 1st frame of a FLI

	TITLE   ubrun

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
	PUBLIC _unbrun
	;unbrun_(cbuf, screen, linect)
_unbrun PROC far
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
	test bl,bl
	jmp endulcloop
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
	add	dx,320
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

_unbrun ENDP

_TEXT	ENDS
END

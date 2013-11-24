;cblock.asm - draw a solid color rectangle on a 320x200 byte-a-pixel screen.
;Will clip if necessary on the spot.  Also routine to do an xor'd rectangle.


	TITLE   cblock
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

	PUBLIC	_getd
;getd(screen, x, y);
_getd	PROC far
	push bp
	mov bp,sp
	push di
	push ds

	mov	ax,[bp+10+2]	;y start
	mov di,320
	mul di
	lds	di,[bp+4+2]	;get screen address
	add	di,ax
	add di,[bp+8+2]   ;fold in x start
	mov	al,[di]
	xor ah,ah
getdz:
	pop ds
	pop di
	pop bp
	ret
_getd	ENDP

_TEXT	ENDS
END

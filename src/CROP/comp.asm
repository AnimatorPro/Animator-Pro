
;comp.asm - low level stuff to uncompress FLI files.

	TITLE   urif

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

	;cset_colors(csource)
	;set the color palette hardware from a compressed source 
	;of format:
	;WORD # of runs, run1, run2, ...,runn
	;each run is of form:
	;BYTE colors to skip, BYTE colors to set, r1,g1,b1,r2,g2,b2,...,rn,gn,bn
	public _cset_colors
_cset_colors proc far
	push bp
	mov bp,sp
	push ds
	push si
	push di
	push cx
	push bx
	cld

	lds si,[bp+4+2]	;load the source compressed color data
	mov di,0		;clear dest color index 
	lodsw
	mov bx, ax   	;get the count of color runs
	test bx,bx
	jmp endcu
cu:
	lodsb		;get the colors to skip
	add di,ax	;add to color index
	lodsb		;get the count of colors to set
	mov cx,ax	;use it as a loop counter
	or  cx,cx	;test for zero
	jnz	set1c
	mov cx,256
set1c:
	mov	dx,3c8h	;point dx to vga color control port
	mov ax,di
	out dx,al	;say which color index to start writing to
	inc di		;bump color index
	inc dx		;point port to vga color data
	;jmp s1		;stall as per IBM VGA tech spec to give hardware time to settle
s1:
	lodsb		;get red component
	out dx,al	;tell the video DAC where it's at
	;jmp s2		;stall some more for poor slow hardware
s2:
	lodsb		;same same with green component
	out dx,al
	;jmp s3
s3:
	lodsb		;same with blue
	out dx,al
	loop set1c

	dec bx
endcu:
	jnz cu

	pop bx
	pop cx
	pop di
	pop si
	pop ds
	pop	bp
	ret	

_cset_colors endp


_TEXT	ENDS
END

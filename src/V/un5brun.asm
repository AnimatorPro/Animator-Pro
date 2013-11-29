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

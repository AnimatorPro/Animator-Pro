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

;parameter offsets
screen equ [bp+4+2]
x	equ [bp+8+2]
y 	equ [bp+10+2]
wid	equ	[bp+12+2]
height equ [bp+14+2]
color	equ [bp+16+2]

clipblock proc near
	;make sure width is positive
	mov	bx,wid
	or	bx,bx	;set flags
	jle	bclipped
	;clip to the right
	mov	ax,x	;get starting dest x
	sub	ax,320  ;if its past right of screen clip nothing left
	jge	bclipped
	add	ax,bx	;ax = portion of right side of block past right of screen
	jl	norclip ;if none of it past go clip another side
	sub	wid,ax	
norclip:	;clip to the left
	mov	ax,x	;get starting dest x
	and	ax,ax		;set flags
	jns	nolclip
	add	wid,ax
	jle	bclipped
	mov	word ptr x,0
nolclip:	;clip off the bottom
	mov	bx,height
	or	bx,bx	;set flags
	jle	bclipped
	mov	ax,y	;get starting dest y
	sub	ax,200
	jge	bclipped
	add	ax,bx
	jl	nobclip
	sub	height,ax
nobclip:	;clip off the top
	mov	ax,y	;get starting dest y
	and	ax,ax		;set flags
	jns	nouclip
	add	height,ax
	jle	bclipped
	mov	word ptr y,0
nouclip:
	clc
	ret
bclipped:
	stc
	ret
clipblock endp

	PUBLIC	_xorblock
;xorblock(screen, x, y, width, height, color);
_xorblock	PROC far
	push bp
	mov bp,sp
	push cx
	push es
	push ds
	push di
	push si
	push bx
	push dx

	call clipblock
	jc xorclipped

	les	bx,screen	;get screen address
	lds	si,screen
	mov	ax,y	;y start
	mov dx,320
	mul dx
	add	bx,ax
	add bx,x   ;fold in x start

	mov ah,color	;get color
	mov dx,height	;get hieght
	jmp	zxb

xb:	mov	cx,wid	;width into count register
	mov	di,bx		;start of line into dest index
	mov	si,bx		;start of line into source index
xp: lodsb
	xor	al,ah
	stosb
	loop	xp
	add bx,320

zxb: dec dx
	js zzxb
	jmp xb

xorclipped:
zzxb:
	pop dx
	pop bx
	pop si
	pop di
	pop ds
	pop es
	pop cx
	pop bp
	ret
_xorblock	ENDP


	PUBLIC	_cblock
;cblock(screen, x, y, width, height, color);
_cblock	PROC far
	push bp
	mov bp,sp
	push cx
	push es
	push di
	push si
	push bx

	les	bx,screen	;get screen address
	mov	ax,y	;y start
	mov si,320
	mul si
	add	bx,ax
	add bx,x   ;fold in x start

	mov al,color	;get color
	mov si,height	;get hieght
	jmp	zcb

cb:	mov	cx,wid	;width into count register
	mov	di,bx		;start of line into dest index
	rep stosb
	add bx,320

zcb: dec si
	js zzcb
	jmp cb

zzcb:
	pop bx
	pop si
	pop di
	pop es
	pop cx
	pop bp
	ret
_cblock	ENDP

	PUBLIC	_chli
;chli(screen, x, y, width, color);
_chli	PROC far
	push bp
	mov bp,sp
	push cx
	push es
	push di

	mov	ax,[bp+10+2]	;y start
	mov di,320
	mul di
	les	di,[bp+4+2]	;get screen address
	add	di,ax
	add di,[bp+8+2]   ;fold in x start
	mov al,[bp+14+2]	;get color
	mov	cx,[bp+12+2]	;width into count register

	rep stosb

	pop di
	pop es
	pop cx
	pop bp
	ret
_chli	ENDP

	PUBLIC	_cvli
;cvli(screen, x, y, height, color);
_cvli	PROC far
	push bp
	mov bp,sp
	push cx
	push es
	push di

	mov	ax,[bp+10+2]	;y start
	mov di,320
	mul di
	les	di,[bp+4+2]	;get screen address
	add	di,ax
	add di,[bp+8+2]   ;fold in x start
	mov al,[bp+14+2]	;get color
	mov	cx,[bp+12+2]	;height into count register

vlp:
	stosb
	add	di,319
	loop	vlp

	pop di
	pop es
	pop cx
	pop bp
	ret
_cvli	ENDP

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
	PUBLIC	_cdot


;cdot(screen, x, y, color);
_cdot	PROC far
	push bp
	mov bp,sp
	push di
	push ds

	mov	ax,[bp+10+2]	;y start
	or ax,ax
	js	cdotz
	cmp	ax,200
	jge	cdotz


	mov di,320
	mul di
	lds	di,[bp+4+2]	;get screen address
	add	di,ax
	mov	dx,[bp+8+2]
	or	dx,dx
	js  cdotz
	cmp dx,320
	jge cdotz
	add di,[bp+8+2]   ;fold in x start
	mov al,[bp+12+2]	;get color
	mov	[di],al
cdotz:
	pop ds
	pop di
	pop bp
	ret
_cdot	ENDP

	PUBLIC	_xordot
;xordot(screen, x, y, color);
_xordot	PROC far
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
	mov al,[di]
	xor al,[bp+12+2]	;get color
	mov	[di],al

	pop ds
	pop di
	pop bp
	ret
_xordot	ENDP

_TEXT	ENDS
END

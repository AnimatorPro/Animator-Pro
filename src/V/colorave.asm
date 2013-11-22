
;colorave.asm - This module computes the rgb average of a pixel and it's 
;8 neighbors...
;:ts=10


	TITLE   colorave
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


;colorave(x,y,rgb,screen,cmap)
x	equ	word ptr[bp+4+2]
y	equ	word ptr[bp+6+2]
rgbo	equ	dword ptr[bp+8+2]
rgbs	equ	word ptr[bp+10+2]
screeno	equ	dword ptr[bp+12+2]
screens	equ	word ptr[bp+14+2]
cmapo	equ	dword ptr[bp+16+2]
cmaps	equ	word ptr[bp+18+2]
	PUBLIC _colorave
_colorave	proc far
	push bp
	mov bp,sp
	sub  sp,27
	push ds
	push si
	push di
	push bx


;make screen address of upper left pixel neighbor in ds:si
	mov ax,y
	dec ax
	mov dx,320
	mul dx
	add ax,x
	dec ax
	lds si,screeno
	add si,ax

;put colormap in es:dx
	les dx,cmapo

	sub bp,27 ;point bp to first bit of table
	sub ah,ah ;get rid of hi byte of ax

;Transfer the rgb values of a 3x3 square surrounding x,y into
;a buffer on the stack...
	mov cx,3
colors27:
	push cx
	mov cx,3
colors3:
	lodsb	;get a pixel
	mov di,dx
	add di,ax	
	add di,ax
	add di,ax ;color address (3 to a color) in di now
	mov bx,es:[di]
	mov [bp],bx
	mov bl,es:[2+di]
	mov [bp+2],bl
	add bp,3
	loop colors3
	pop cx
	add si,(320-3)
	loop colors27

	;see if 0 or less ... fix up top line if needed
	mov bx,y
	dec bx
	jns ok1

	mov si,ss
	mov es,si
	mov ds,si
	mov si,bp
	sub si,18
	mov di,bp
	sub di,27
	mov cx,9
	rep movsb
ok1:
	sub bx,198
	js ok2
	mov si,ss
	mov es,si
	mov ds,si
	mov si,bp
	sub si,18
	mov di,bp
	sub di,9
	mov cx,9
	rep movsb
ok2:

	les di,rgbo ;point es:di to final rgb result buffer
	sub bp,3	;use bp to index unpacked components 1 at a time
	mov ch,3	;and there's three components with ch a loop counter
	sub bh,bh	;zero top of bx
component:
	;accumulate result in ax...
	mov bl,[bp] ;ah is still == 0
	mov ax,bx	
	mov bl,[bp-3]
	add ax,bx
	add ax,bx
	mov bl,[bp-6]
	add ax,bx
	mov bl,[bp-9]
	add ax,bx
	add ax,bx
	mov bl,[bp-12] ;the center pixel ... 4x
	add bl,bl	     ;no overflow since component 0-63
	add ax,bx
	add ax,bx
	mov bl,[bp-15]
	add ax,bx
	add ax,bx
	mov bl,[bp-18]
	add ax,bx
	mov bl,[bp-21]
	add ax,bx
	add ax,bx
	mov bl,[bp-24]
	add ax,bx
	add ax,8	;round it before the divide by 16
	mov cl,4
	shr ax,cl
	stosb	;save the result
	add bp,1	;move bp to next component
	dec ch	;using ch as a loop counter
	jnz component


	pop bx
	pop di
	pop si
	pop ds
	mov sp,bp
	pop bp
	ret
_colorave	endp

_TEXT	ENDS
END

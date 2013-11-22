;diag.asm - map a diagonal line to a horizontal line.  The core of
;the raster rotating/stretching machinery in Vpaint.

	TITLE  diag 
DIAG_TEXT	SEGMENT  BYTE PUBLIC 'CODE'
DIAG_TEXT	ENDS
_DATA	SEGMENT  WORD PUBLIC 'DATA'
_DATA	ENDS
CONST	SEGMENT  WORD PUBLIC 'CONST'
CONST	ENDS
_BSS	SEGMENT  WORD PUBLIC 'BSS'
_BSS	ENDS
DGROUP	GROUP	CONST,	_BSS,	_DATA
	ASSUME  CS: DIAG_TEXT, DS: DGROUP, SS: DGROUP, ES: DGROUP
DIAG_TEXT      SEGMENT

;diag_to_table(s,sbpr,dtable,dsize,x0,y0,x1,y1)
;register PLANEPTR s;
;PLANEPTR dtable;
;WORD sbpr;
;register WORD dsize;
;WORD x0,y0,x1,y1;
	public _dto_table
_dto_table proc far
s	equ	[bp+6]		;source cel image
sbpr equ [bp+10]	;distance between lines of cel
dtab equ [bp+12]	;where to output pixels
dsize equ [bp+16]	;# of pixels to output
x0	equ [bp+18]
y0	equ [bp+20]
x1  equ [bp+22]
y1  equ [bp+24]
	push bp
	mov bp,sp
	sub sp,16	;local space
incx equ [bp-2]
incy equ [bp-4]
deltax equ [bp-6]
deltay equ [bp-8]

	push bx
	push cx
	push si
	push di
	push ds
	push es

	lds si,s	;source in ds:si
	les di,dtab ;dest in es:di

	;compute address of x0/y0 into ds:si
	mov ax,y0
	mov dx,sbpr
	mul dx
	add si,ax	;add in y component
	mov ax,x0   ;fetch x
	add si,ax   ;add in x component

	;compute deltax and incx
	mov dx,-1  ;dx = incx
	mov bx,x1
	sub bx,ax  ;bx = x1-x0
	js nnx
	neg dx
	neg bx
nnx: mov incx,dx
	mov deltax,bx  ;deltax = -|x1-x0|

	;compute deltay and incy
	mov dx,sbpr  ;dx = incy
	neg dx
	mov bx,y1
	sub bx,y0  ;bx = y1-y0
	js nny
	neg dx
	neg bx
nny: mov incy,dx
	mov deltay,bx	;deltay = -|y1-y0|

	mov dx,dsize ;get dsize into handy register for inner loop

	;ax = xerr = dsize + deltax/2
	mov ax,deltax
	sar ax,1
	add ax,dx

	;bx = yerr = dsize + deltay/2
	mov bx,deltay
	sar bx,1
	add bx,dx

	mov cx,dx	;dot count into dx

	dec word ptr deltax
	dec word ptr deltay

innerlp:
	movsb	;move that babe!
	dec	si	;oops didn't mean to increment source here

	add ax,deltax
	jg	ystep
nextx: add si,incx
	add ax,dx
	jle nextx

ystep:
	add bx,deltay
	jg nextlp
nexty: add si,incy
	add bx,dx
	jle nexty

nextlp:
	loop innerlp

	pop es
	pop ds
	pop di
	pop si
	pop cx
	pop bx
	mov sp,bp
	pop bp
	ret
_dto_table endp
DIAG_TEXT ENDS
END

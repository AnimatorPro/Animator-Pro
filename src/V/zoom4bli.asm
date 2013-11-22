;zoom4bli.asm - zoom an area of the screen x 4.  Does not HAVE to be
;a full screen width section, but there are some restrictions on width
;of dest, and full screen is only way used in Vpaint.  SUPPOSED to
;deal with arbitrary heights.  I suspect there's a lurking bug with
;non divisible by 4 heights.

;:ts=10

	TITLE   zoom4blit
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




	PUBLIC	_zoom4blit
;zoomblit(wid,height,sx1,sy1,spt,snext_line,dx1,dy1,dpt,dnext_line)
;zoomblit(160,100,2,80,50,vf.p,vf.bpr,0,0,sf.p,sf.bpr);
wid	equ 6
height	equ 8
sx1	equ 10
sy1	equ 12
spt_o	equ 14
spt_s	equ 16
snext	equ 18
dx1	equ 20
dy1	equ 22
dpt_o	equ 24
dpt_s	equ 26
dnext	equ 28
minusex	equ word ptr[bp-2]
_zoom4blit	PROC far
	push bp
	mov bp,sp
	push ds
	push si
	push di
	push bx

	;get starting source address in ds:si
	lds	si,[bp+spt_o]	
	mov	ax,[bp+sy1]
	mov	bx,[bp+snext]
	mul	bx
	add	si,ax
	add	si,[bp+sx1]

	;get starting destination address in es:di
	les	di,[bp+dpt_o]	
	mov	ax,[bp+dy1]
	mov	bx,[bp+dnext]
	mul	bx
	add	di,ax
	add	di,[bp+dx1]

	mov 	dx,si
	mov bx,[bp+wid]
	add bx,7
	mov cl,3
	shr bx,cl	;8 at a time

	mov ax,[bp+height]
	mov cx,[bp+dy1]
	;or cx,cx			;say what????
	and cx,3
	jz center
	neg cx
	add cx,4
top:
	push cx
	call hafline
	pop cx
	dec word ptr[bp+height]	;dec height
	loop top

	add dx,320	;go to next line of source
center:
	mov ax,[bp+height]
	push ax		;save old height
	shr ax,1		
	shr ax,1		
	mov [bp+height],ax	;save scaled height

	jmp zzblp
blp:
	call hafline
	call hafline
	call hafline
	call hafline
	add dx,320	;go to next line of source
zzblp:
	dec word ptr[bp+height]
	jns blp

	pop cx
	and cx,3
	jz za1
bottom:
	push cx
	call hafline
	pop cx
	loop bottom
za1:

	pop bx
	pop di
	pop si
	pop ds
	mov sp,bp
	pop bp
	ret

_zoom4blit	ENDP

hafline	proc near
	mov si,dx
	mov cx,bx	;# of 8's to do
do8:
	lodsb	;1
	stosb
	stosb
	stosb
	stosb
	lodsb	;2
	stosb
	stosb
	stosb
	stosb
	lodsb	;3
	stosb
	stosb
	stosb
	stosb
	lodsb	;4
	stosb
	stosb
	stosb
	stosb
	lodsb	;5
	stosb
	stosb
	stosb
	stosb
	lodsb	;6
	stosb
	stosb
	stosb
	stosb
	lodsb	;7
	stosb
	stosb
	stosb
	stosb
	lodsb	;8
	stosb
	stosb
	stosb
	stosb
	loop do8
	ret
hafline	endp



_TEXT	ENDS
END

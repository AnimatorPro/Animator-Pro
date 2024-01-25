
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
MYINT	equ	8H

	;clock up interrupt frequency to close to 80 Hz 
	public _fastint	
_fastint proc far
	cli
	;set timer 0 to 80Hz
	mov	al,66
	mov dx,40H
	out dx,al
	out dx,al
	mov cs:word ptr skipdos,3
	sti
	ret
_fastint endp

	;install this here interrupt routine
	public _setint
_setint proc	far
	push bp
	mov bp,sp
	push es
	push ax
	push bx

	;extra segment to interrupt table
	mov	ax,0
	mov	es,ax
	;get old interrupt in axbx and save it in code segment
	cli
	mov ax,es:word ptr[MYINT*4]
	mov bx,es:word ptr[MYINT*4+2]
	mov cs:oldint,ax
	mov cs:oldint+2,bx
	mov es:word ptr[MYINT*4],offset _int
	mov es:word ptr[MYINT*4+2],seg _int
	sti

	pop bx
	pop ax
	pop es
	pop	bp
	ret
_setint endp

	; clear out this interrupt routine
	public _Restoreint
_Restoreint proc far
	push bp
	mov bp,sp
	push es
	push ax
	push bx

	;extra segment to interrupt table
	mov	ax,0
	mov	es,ax
	;restore old interrupt from code segment
	cli
	mov ax,cs:oldint
	mov es:word ptr[MYINT*4],ax
	mov ax,cs:oldint+2
	mov es:word ptr[MYINT*4+2],ax
	;set timer 0 to 18Hz
	mov al,255
	mov dx,40H
	out dx,al
	out dx,al
	sti

	pop bx
	pop ax
	pop es
	pop	bp
	ret
_Restoreint endp


	;my very own timer interrupt service routine.
_int	proc	far
	inc cs:word ptr iclock
	jnz nocry
	inc cs:word ptr iclock+2
nocry:
	dec cs:word ptr nownthen
	js dosint	;every now and then have to pass interrupt back to dos...
	push ax
	mov ax,20h
	out 20h,ax	;send a 20 to port 20 to clear the interrupt (thanks TJ!)
	pop ax
	iret

dosint:	
	push ax
	mov ax,cs:word ptr skipdos
	mov cs:word ptr nownthen,ax
	pop ax
	jmp cs:dword ptr oldint
_int	endp



	public _get80hz
_get80hz proc far
	mov ax,cs:iclock
	mov dx,cs:iclock+2
	ret
_get80hz endp


oldint equ this word
	db 1
	db 2
	db 3
	db 4

iclock equ this word
	dw 0
	dw 0

nownthen equ this word
	dw 1
skipdos equ this word
	dw 1

	;check on vblank status
	public _pollvstatus
_pollvstatus proc far
	mov	dx,3dah
	xor ah,ah
	in al,dx
	ret
_pollvstatus endp

	;busy-wait for vblank
	public _wait_vblank
_wait_vblank proc far
	mov	dx,3dah	;video status port
wvb:
	in	al,dx
	test al,8
	jz wvb
	ret
_wait_vblank endp

	;busy-wait for out of vblank
	public _wait_novblank
_wait_novblank proc far
	mov	dx,3dah	;video status port
wnvb:
	in	al,dx
	test al,8
	jnz wnvb
	ret
_wait_novblank endp

	;set the old color map
	public _jset_colors
_jset_colors proc far
	;jset_colors(0, 256, vga_cmap);
	push bp
	mov bp,sp
	push cx
	push bx
	push ds
	push si

	cld
	mov	bx,[bp+4+2]
	mov	cx,[bp+6+2]
	lds	si,[bp+8+2]

st1:
	mov	dx,3c8h
	mov al,bl
	out dx,al
	inc bl
	inc dx
	jmp s1
s1:
	lodsb
	out dx,al
	jmp s2
s2:
	lodsb
	out dx,al
	jmp s3
s3:
	lodsb
	out dx,al
	loop st1


	pop si
	pop ds
	pop bx
	pop cx
	pop bp
	ret
_jset_colors endp

	;set a single color
	public _cpoke
_cpoke proc far
	push bp
	mov bp,sp
	mov dx,3c8h
	xor al,al
	out dx,al
	inc dx
	mov al,[bp+4+2]
	jmp j1
j1:
	out dx,al
	jmp j2
j2:
	out dx,al
	jmp j3
j3:
	out dx,al

	pop bp
	ret
_cpoke endp




_TEXT	ENDS
END

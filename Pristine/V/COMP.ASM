;comp.asm - some C glue to the 8086's string (repl XXX) instructions.  These
;are used extensively by the compression side of the FLI machinery.  Following
;these are some routines for decompressing FLI delta frames.

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
	PUBLIC _bsame
	;bsame(d, count)
_bsame PROC far
	push bp
	mov bp,sp
	push di
	cld

	les di,[bp+4+2]
	mov cx,[bp+8+2]
	mov ax,es:[di]
	inc cx
	repe scasb

	mov ax,[bp+8+2]
	sub ax,cx

	pop di
	pop	bp
	ret	
_bsame ENDP


	PUBLIC _fsame
	;fsame(d, count)
_fsame PROC far
	push bp
	mov bp,sp
	push di
	cld

	les di,[bp+4+2]
	mov cx,[bp+8+2]
	mov ax,es:[di]
	inc cx
	repe scasw

	mov ax,[bp+8+2]
	sub ax,cx

	pop di
	pop	bp
	ret	
_fsame ENDP

	PUBLIC _bcontrast
	;bcontrast(s1, s2, count)
_bcontrast PROC far
	push bp
	mov bp,sp
	push ds
	push si
	push di
	cld

	lds si,[bp+4+2]
	les di,[bp+8+2]
	mov cx,[bp+12+2]
	repne cmpsb
	inc cx
	mov ax,[bp+12+2]
	sub ax,cx

	pop di
	pop si
	pop ds
	pop	bp
	ret	
_bcontrast ENDP


	PUBLIC _fcontrast
	;fcontrast(s1, s2, count)
_fcontrast PROC far
	push bp
	mov bp,sp
	push ds
	push si
	push di
	cld

	lds si,[bp+4+2]
	les di,[bp+8+2]
	mov cx,[bp+12+2]
	repne cmpsw
	inc cx

	mov ax,[bp+12+2]
	sub ax,cx

	pop di
	pop si
	pop ds
	pop	bp
	ret	
_fcontrast ENDP

	PUBLIC _bcompare
	;bcompare(s1,s2,count)
_bcompare PROC far
	push bp
	mov bp,sp
	push ds
	push si
	push di
	cld

	lds si,[bp+4+2]
	les di,[bp+8+2]
	mov cx,[bp+12+2]
	inc cx
	repe cmpsb

	mov ax,[bp+12+2]
	sub ax,cx

	pop di
	pop si
	pop ds
	pop	bp
	ret	

_bcompare ENDP

	PUBLIC _fcompare
	;fcompare(s1, s2, count)
_fcompare PROC far
	push bp
	mov bp,sp
	push ds
	push si
	push di
	cld

	lds si,[bp+4+2]
	les di,[bp+8+2]
	mov cx,[bp+12+2]
	inc cx
	repe cmpsw

	mov ax,[bp+12+2]
	sub ax,cx

	pop di
	pop si
	pop ds
	pop	bp
	ret	

_fcompare ENDP


	PUBLIC _unrun
	;unrun(cbuf, screen)
_unrun PROC far
	push bp
	mov bp,sp
	push ds
	push si
	push di
	push cx
	push bx
	cld

	lds si,[bp+4+2]
	les di,[bp+8+2]
	lodsw
	mov bx, ax   ;get the count
	test bx,bx
	jmp endunloop
unloop:
	lodsw
	test ax,ax	;check sign
	js copy
	mov cx,ax
	lodsw
	rep stosw
	dec bx
	jnz	unloop
	jmp endunrun

copy:
	neg ax
	mov cx,ax
	rep movsw
	dec bx
endunloop:
	jnz unloop

endunrun:
	pop bx
	pop cx
	pop di
	pop si
	pop ds
	pop	bp
	ret	

_unrun ENDP


	PUBLIC _unsbsrsccomp
	;unsbrsccomp_(cbuf, screen)
_unsbsrsccomp PROC far
	push bp
	mov bp,sp
	push es
	push ds
	push si
	push di
	push bx
	push cx
	cld

	lds si,[bp+4+2]
	les di,[bp+8+2]
	lodsw	;get the first skip (WORD)
	add di,ax
	lodsw		;get op count
	mov bx, ax  
	xor ah,ah
	test bx,bx
	jmp endusbsrscloop
usbsrscloop:
	lodsb	;load in the byte skip
	add di,ax
	lodsb	; load op/count
	test al,al
	js usbsrscrun
	mov cx,ax
	rep movsb
	dec bx
	jnz usbsrscloop
	jmp usbsrscout
usbsrscrun:
	neg al
	mov cx,ax ;get signed count
	lodsb	  ;value to repeat in al
	rep stosb
	dec bx
endusbsrscloop:
	jnz usbsrscloop
usbsrscout:
	pop cx
	pop bx
	pop di
	pop si
	pop ds
	pop es
	pop	bp
	ret	

_unsbsrsccomp ENDP


	PUBLIC _unlccomp
	;unlccomp_(cbuf, screen)
_unlccomp PROC far
linect equ word ptr[bp-2]
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
	lodsw	;get the count of # of lines to skip
	mov dx,320
	mul dx
	add di,ax
	lodsw		;get line count
	mov	linect,ax	;save it on stack
	mov	dx,di	;keep pointer to start of line in dx
	xor	ah,ah	;clear hi bit of ah cause lots of unsigned uses to follow
linelp:
	mov	di,dx
	lodsb		;get op count for this line
	mov bl,al  
	test bl,bl
	jmp endulcloop
ulcloop:
	lodsb	;load in the byte skip
	add di,ax
	lodsb	; load op/count
	test al,al
	js ulcrun
	mov cx,ax
	rep movsb
	dec bl
	jnz ulcloop
	jmp ulcout
ulcrun:
	neg al
	mov cx,ax ;get signed count
	lodsb	  ;value to repeat in al
	rep stosb
	dec bl
endulcloop:
	jnz ulcloop
ulcout:
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

_unlccomp ENDP

	public _fcuncomp
	;uncompress color palette onto a buffer
_fcuncomp proc far
	push bp
	mov bp,sp
	push ds
	push si
	push di
	push cx
	push bx
	cld

	lds si,[bp+4+2]
	les di,[bp+8+2]
	lodsw
	mov bx, ax   ;get the count
	test bx,bx
	jmp endu
u:
	lodsb		;get colors to skip
	add di,ax
	add di,ax
	add di,ax
	lodsb		;get colors to copy
	or  al,al	;test for zero
	jnz	u2
	mov ax,256
u2:
	mov cx,ax
	add cx,ax
	add cx,ax
	rep movsb
	dec bx
endu:
	jnz u

	pop bx
	pop cx
	pop di
	pop si
	pop ds
	pop	bp
	ret	

_fcuncomp endp

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

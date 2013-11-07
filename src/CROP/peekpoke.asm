;peekpoke.asm - 
;This module is really pretty boring.  Mostly ways to copy memory
;fairly quickly, and my sysint routine (which follows aztec conventions)
;'cause I couldn't find one in Microsoft.
	TITLE   peekpoke

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


	PUBLIC _sysint
	;sysint(interrupt, inregs, outregs)
	;Does a software interrupt from C.
	;Returns flags in ax
	;interrupt is 16 bit saying which interrupt to generate.
	;inregs and outregs are the following structure:
	;struct byte_regs 
	;	{
	;	unsigned char al, ah, bl, bh, cl, ch, dl, dh;
	;	unsigned int si, di, ds, es;
	;	};
	;Inregs and outregs may usually point to the same structure
	;This generates a warning during assembly but works ok.
_sysint PROC far
	push bp
	mov bp,sp
	push bx
	push cx
	push si
	push di
	push es
	push ds

	;grab interrupt number and use it to modify intit code  (no ROM for me!)
	mov ax,[bp+4+2]
	mov byte ptr cs:intit+1,al

	;point ds:di to input parameters
	lds di,[bp+6+2]
	mov ax,[di]
	mov bx,[di+2]
	mov cx,[di+4]
	mov dx,[di+6]
	mov si,[di+8]
	push ax
	mov ax,[di+14]
	mov es,ax
	mov ax,ss
	mov cs:oss,ax
	mov cs:osp,sp
	pop ax
	lds di,[di+10]
intit:
	int 0
	cli
	mov cs:oax,ax
	mov ax,cs:oss
	mov ss,ax
	mov sp,cs:osp
	sti
	pop ax	;
	mov ax,cs:oax
	;save ds:di and point 'em to output parameters
	push ds
	push di
	lds di,[bp+10+2]
	mov [di],ax
	mov [di+2],bx
	mov [di+4],cx
	mov [di+6],dx
	mov [di+8],si
	pop ax	;'di' into ax
	mov [di+10],ax
	pop ax	;'ds' into ax
	mov [di+12],ax
	mov ax,es
	mov [di+14],ax

	;move flags to ax (the return value...)
	pushf	
	pop ax

	pop ds
	pop es
	pop di
	pop si
	pop cx
	pop bx
	pop bp
	ret
oax equ this word
	dw 0
oss equ this word
	dw 0
osp equ this word
	dw 0
_sysint endp


	PUBLIC	_norm_pointer
	;norm_pointer(offset, seg)
	;Add as much as possible of the offset of a pointer to the segment
_norm_pointer	PROC far
	push bp
	mov bp,sp
	push cx

	mov	ax,[bp+4+2]	;offset
	mov dx,[bp+6+2]   ;segment
	mov cl,4
	shr ax,cl
	add dx,ax
	mov	ax,[bp+4+2]	;offset
	and ax,15

	pop cx
	pop	bp
	ret	
_norm_pointer	ENDP

	PUBLIC	_enorm_pointer
	;norm_pointer(offset, seg)
	;Add as much as possible of the offset of a pointer to the segment
	;and make it evenly alligned...
_enorm_pointer	PROC far
	push bp
	mov bp,sp
	push cx

	mov	ax,[bp+4+2]	;offset
	mov dx,[bp+6+2]   ;segment
	inc ax
	and ax,0FFFEh	;force even allignment
	mov [bp+4+2],ax ;and save...
	mov cl,4
	shr ax,cl
	add dx,ax
	mov	ax,[bp+4+2]	;offset
	and ax,15

	pop cx
	pop	bp
	ret	
_enorm_pointer	ENDP

;stuff_words(data, offset, seg, words);
	PUBLIC	_stuff_words
_stuff_words	PROC far
	push	bp
	mov	bp,sp
	push es
	push di
	push cx
	mov	ax,[bp+8+2]	;seg
	mov	di,[bp+6+2]	;offset
	mov es,ax
	mov ax,[bp+4+2]   ;value to poke
	mov cx,[bp+10+2]  ;count
	cld
	rep stosw
	pop cx
	pop di
	pop es
	pop	bp
	ret	
_stuff_words	ENDP

;xor_words(data, offset, seg, words/8);
	PUBLIC	_xor_words
_xor_words	PROC far
	push	bp
	mov	bp,sp
	push ds
	push es
	push bx
	push cx

	mov	ax,[bp+8+2]	;seg
	mov	bx,[bp+6+2]	;offset
	mov ds,ax
	mov es,ax
	mov ax,[bp+4+2]   ;value to xor
	mov cx,[bp+10+2]  ;count
ook:
	xor [bx],ax
	xor [bx+2],ax
	xor [bx+4],ax
	xor [bx+6],ax
	xor [bx+8],ax
	xor [bx+10],ax
	xor [bx+12],ax
	xor [bx+14],ax
	add bx,16
	loop ook

	pop cx
	pop bx
	pop es
	pop ds
	pop	bp
	ret	
_xor_words	ENDP

;copy_bytes(s,d,bytes)
	public _copy_bytes
_copy_bytes proc far
	push bp
	mov bp,sp
	push ds
	push si
	push di

	cld
	lds	si,[4+bp+2]
	les	di,[8+bp+2]
	mov cx,[12+bp+2]
	rep movsb

	pop di
	pop si
	pop ds
	pop bp
	ret
_copy_bytes endp


;copy_structure(soffset, ssegment, doffset, dsegment, bytes)
    PUBLIC _copy_structure
_copy_structure PROC far
	push	bp
	mov	bp,sp
	push di
	push ds
	push es
	push si
	push cx

	mov	ax,[bp+6+2]	;s seg
	mov ds,ax
	mov	si,[bp+4+2]	;s offset
	mov ax,[bp+10+2]  ;d seg
	mov es,ax
	mov di,[bp+8+2]  ;d offset
	mov cx,[bp+12+2]  ;count
	shr cx,1

	cld
	rep movsw

	pop cx
	pop si
	pop es
	pop ds
	pop di
	pop	bp
	ret	
_copy_structure ENDP


;exchange_words(s, d, words)
	public _exchange_words
_exchange_words proc far
	push bp
	mov bp,sp
	push es
	push ds
	push si
	push di
	push cx

	les	di,[bp+8+2]
	lds si,[bp+4+2]
	mov	cx,[bp+12+2]
exglp:
	mov	ax,[si]
	mov dx,es:[di]
	mov [si],dx
	inc si
	inc si
	stosw
	loop exglp
	pop cx
	pop di
	pop si
	pop ds
	pop es
	pop bp
	ret
_exchange_words endp

;copy_words(soffset, ssegment, doffset, dsegment, words)
	PUBLIC	_copy_words
_copy_words	PROC far
	push	bp
	mov	bp,sp
	push es
	push di
	push ds
	push si
	push cx

	mov	ax,[bp+6+2]	;s seg
	mov ds,ax
	mov	si,[bp+4+2]	;s offset
	mov ax,[bp+10+2]  ;d seg
	mov es,ax
	mov di,[bp+8+2]  ;d offset
	mov cx,[bp+12+2]  ;count

	cld
	rep movsw

	pop cx
	pop si
	pop ds
	pop di
	pop es
	pop	bp
	ret	
_copy_words	ENDP


	PUBLIC _xlat
	;xlat(table, buf, count)
	;table -> 256 byte translation table
	;buf -> area of count bytes to translate
_xlat proc far
	push bp
	mov bp,sp
	push bx
	push cx
	push di
	push ds
	push es

	lds bx,[2+4+bp]	;load ds:bx with table
	les di,[2+8+bp]	;load es:di with buffer
	mov cx,[2+12+bp]

xllp:
	mov al,es:[di]	;fetch a byte
	xlat [2+4+bp]
	stosb			;and store result
	loop	xllp

	pop es
	pop ds
	pop di
	pop cx
	pop bx
	pop bp
	ret
_xlat endp





	public _back_scan
_back_scan	proc	far
	push	bp
	mov		bp,sp
	push	di
	push	es

	les	di,[bp+6+2]
	dec di
	mov cx,[bp+10+2]
	mov al,[bp+4+2]
	std
	rep scasb
	cld
	inc cx
	mov ax,[bp+10+2]
	sub ax,cx

	pop	es
	pop	di
	pop	bp
	ret
_back_scan endp

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
	inc cx
	repne cmpsw

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


_TEXT	ENDS
END

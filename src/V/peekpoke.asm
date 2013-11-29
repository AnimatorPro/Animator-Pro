;peekpoke.asm - This module is really pretty boring.  Mostly ways to copy memory
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


;xor_group(s, d, words)
	public _xor_group
_xor_group proc far
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
xorglp:
	lodsw
	mov dx,es:[di]
	xor ax,dx
	stosw
	loop xorglp
	pop cx
	pop di
	pop si
	pop ds
	pop es
	pop bp
	ret
_xor_group endp


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

_TEXT	ENDS
END

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





_TEXT	ENDS
END

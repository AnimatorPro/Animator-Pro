;*****************************************************************************
;* strfuncs.asm - Because aa_syslib doesn't provide all the string utils.
;*****************************************************************************

	include stdmacro.i

_TEXT	segment

	public	strchr
	public	stricmp
	public	strstr
	public	stristr

;-----------------------------------------------------------------------------
; static void copylower - copy a string, lowercasing it as it's moved.
; Entry:
;   ecx - source
;   edx - dest
; Exit:
;   eax,ecx,edx - trashed
;-----------------------------------------------------------------------------

copylower proc near

#loop:
	mov	al,[ecx]
	cmp	al,'A'
	jb	short #have_it
	cmp	al,'Z'
	ja	short #have_it
	or	al,020h
#have_it:
	mov	[edx],al
	inc	ecx
	inc	edx
	test	al,al
	jnz	short #loop
	ret

copylower endp

;*****************************************************************************
;* char *strchr(char *str, char chr);
;*****************************************************************************

strchr	proc	near

	mov	eax,[esp+4]
	mov	edx,[esp+8]
#loop:
	mov	cl,[eax]
	cmp	cl,dl
	je	short #return
	inc	eax
	test	cl,cl
	jnz	short #loop
	xor	eax,eax
#return:
	ret

strchr	endp

;*****************************************************************************
;* int stricmp(char *str1, char *str2);
;*****************************************************************************

stricmp proc	near

	mov	ecx,[esp+4]
	mov	edx,[esp+8]
	push	ebx
#loop:
	mov	al,[ecx]
	cmp	al,'A'
	jb	short #have_a
	cmp	al,'Z'
	ja	short #have_a
	or	al,020h
#have_a:
	mov	bl,[edx]
	cmp	bl,'A'
	jb	short #have_b
	cmp	bl,'Z'
	ja	short #have_b
	or	bl,020h
#have_b:
	test	al,al
	jz	short #atend
	test	bl,bl
	jz	short #atend
	sub	al,bl
	jnz	short #notequal
	inc	ecx
	inc	edx
	jmp	short #loop
#atend:
	sub	al,bl
#notequal:
	pop	ebx
	movsx  eax,al
	ret

stricmp endp

;*****************************************************************************
;* char *strstr(char *str, char *substr);
;*****************************************************************************

strstr	proc	near

	mov	eax,[esp+4]
	mov	edx,[esp+8]

_stristr proc	near

#mainloop:
	mov	cl,[eax]
	test	cl,cl
	jz	short #notfound
	cmp	cl,[edx]
	je	short #checksubstr
	inc	eax
	jmp	short #mainloop
#checksubstr:
	push	eax
	push	edx
#subloop:
	inc	eax
	inc	edx
	mov	cl,[edx]
	test	cl,cl
	jz	short #subequal
	cmp	cl,[eax]
	je	short #subloop
	pop	edx
	pop	eax
	inc	eax
	jmp	short #mainloop
#subequal:
	pop	edx
	pop	eax
	jmp	short #return
#notfound:
	xor	eax,eax
#return:
	ret

_stristr endp
strstr	endp

;*****************************************************************************
;* char *stristr(char *str, char *substr);
;*****************************************************************************

stristr proc near

#str	 equ	[ebp+8]
#substr  equ	[ebp+12]
#lstr	 equ	[ebp-1024]
#lsubstr equ	[ebp-512]

	push	ebp
	mov	ebp,esp
	sub	esp,1024

	mov	ecx,#str
	lea	edx,#lstr
	call	copylower
	mov	ecx,#substr
	lea	edx,#lsubstr
	call	copylower

	lea	eax,#lstr
	lea	edx,#lsubstr
	call	_stristr

	leave
	ret

stristr endp


_TEXT	ends
	end


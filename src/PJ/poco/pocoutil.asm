;*****************************************************************************
;* POCOUTIL.ASM - Little assembler routines to help speed up poco.
;*****************************************************************************

_DATA	segment public dword use32 'data'

istab:
	db	080H, 080H, 080H, 080H, 080H, 080H, 080H, 080H
	db	080H, 090H, 090H, 090H, 090H, 090H, 080H, 080H
	db	080H, 080H, 080H, 080H, 080H, 080H, 080H, 080H
	db	080H, 080H, 080H, 080H, 080H, 080H, 080H, 080H
	db	010H, 020H, 020H, 020H, 020H, 020H, 020H, 020H
	db	020H, 020H, 020H, 020H, 020H, 020H, 020H, 020H
	db	042H, 042H, 042H, 042H, 042H, 042H, 042H, 042H
	db	042H, 042H, 020H, 020H, 020H, 020H, 020H, 020H
	db	020H, 044H, 044H, 044H, 044H, 044H, 044H, 004H
	db	004H, 004H, 004H, 004H, 004H, 004H, 004H, 004H
	db	004H, 004H, 004H, 004H, 004H, 004H, 004H, 004H
	db	004H, 004H, 004H, 020H, 020H, 020H, 020H, 021H
	db	020H, 048H, 048H, 048H, 048H, 048H, 048H, 008H
	db	008H, 008H, 008H, 008H, 008H, 008H, 008H, 008H
	db	008H, 008H, 008H, 008H, 008H, 008H, 008H, 008H
	db	008H, 008H, 008H, 020H, 020H, 020H, 020H, 080H
	rept 128
	db	0
	endm

_CTb	 =	001H				   ; underbar, used by iscsym()
_CTd	 =	002H				   ; numeric digit
_CTu	 =	004H				   ; upper case
_CTl	 =	008H				   ; lower case
_CTs	 =	010H				   ; whitespace
_CTp	 =	020H				   ; punctuation
_CTx	 =	040H				   ; hexadecimal
_CTc	 =	080H				   ; control character

ISALNUM  =	_CTu OR _CTl OR _CTd
ISALPHA  =	_CTu OR _CTl
ISCNTRL  =	_CTc
ISDIGIT  =	_CTd
ISGRAPH  =	_CTd OR _CTu OR _CTl OR _CTp
ISLOWER  =	_CTl
ISPUNCT  =	_CTp
ISSPACE  =	_CTs
ISUPPER  =	_CTu
ISXDIGIT =	_CTx
ISCSYM	 =	_CTu OR _CTl OR _CTb OR _CTd
ISCSYMF  =	_CTu OR _CTl OR _CTb

_DATA	ends

_CODE	segment public dword use32 'code'
	assume	cs:_CODE,ds:_DATA

	public	po_ptr2ppt
	public	po_ppt2ptr
	public	po_skip_space
	public	po_cmatch_scan
	public	po_hashfunc
	public	poco_stuff_bytes
	public	poco_zero_bytes
	public	poco_copy_bytes
	public	po_eqstrcmp
	public	po_chop_csym
	public	__po_chop_csym__
	public	__poco_zero_bytes__
	public	__poco_copy_bytes__

;*****************************************************************************
;* Popot po_ptr2ppt(void *ptr, int bytes)
;*
;*  convert a C pointer and a count of bytes it points to into a poco 12-byte
;*  protected pointer.	this assumes watcom C '3s' calling conventions: on
;*  entry the input parms are on the stack, and [esi] points to the Popot
;*  structure into which we are to place our return value.
;*  assumes Popot is typedef'd as {void *pt,*min,*max}.
;*  this function exists mainly for use by poe modules.
;*****************************************************************************

po_ptr2ppt proc near

	xchg	esi,edi 		; watcom gives us retval ptr in esi,
	mov	eax,[esp+4]		; we need it in edi. get ptr parm from
	stosd				; stack, store it to popot.pt and to
	stosd				; popot.min.  add to it the byte count
	add	eax,[esp+8]		; parm from the stack, store that as
	stosd				; popot.max.
	mov	edi,esi 		; restore edi register and
	ret				; return to caller.

po_ptr2ppt endp

;*****************************************************************************
;* void *po_ppt2ptr(Popot ppt)
;*
;*  convert a 12-byte poco protected pointer to a standard C pointer.
;*  this function exists mainly for use by poe modules.
;*  yes, this function is trivial, but it allows a certain grace in coding
;*  poe modules.  for example, one can code something like:
;*    void *sscreen = ppt2ptr(poeGetSwapScreen());
;*  instead of needing a temporary Popot declared to hold the retval from
;*  poeGetSwapScreen.
;*****************************************************************************

po_ppt2ptr proc near
	mov	eax,[esp+4]
	ret
po_ppt2ptr endp

;*****************************************************************************
;* char    *po_skip_space(char *line)
;*****************************************************************************

po_skip_space	proc near

	mov	eax,[esp+4]		;
	test	eax,eax 		;
	jz	short #return		;
	xor	ecx,ecx 		;
#loop:					;
	mov	cl,[eax]		;
	test	cl,cl			;
	jz	short #retnull		;
	test	byte ptr[ecx+istab],_CTs;
	jz	short #return		;
	inc	eax			;
	jmp	short #loop		;
#retnull:				;
	xor	eax,eax 		;
#return:				;
	ret				;

po_skip_space	endp

;*****************************************************************************
;* char *po_cmatch_scan(char *line)
;*   scan a line of C source code for characters that come in matching pairs:
;*     ' " /  (the latter being part of /* */ pairs).
;*****************************************************************************

po_cmatch_scan proc near

	mov	eax,[esp+4]
	xor	ecx,ecx 		;
#loop:					;
	mov	cl,[eax]		;
	inc	eax
	test	cl,cl			;
	jz	short #retnull		;
	test	byte ptr[ecx+istab],_CTp; quickly eliminate non-punctuation
	jz	short #loop		;
	cmp	cl,022H
	je	short #return
	cmp	cl,027H
	je	short #return
	cmp	cl,02fH
	je	short #return
	jmp	short #loop
#retnull:
	mov	eax,1
#return:
	dec	eax
	ret

po_cmatch_scan endp


;*****************************************************************************
;* int po_hashfunc(char *s)
;*****************************************************************************

po_hashfunc proc near

	xor	ecx,ecx
	mov	edx,[esp+4]
	movzx	eax,byte ptr [edx]
#loop:
	inc	edx
	mov	cl,[edx]
	add	eax,ecx
	test	cl,cl
	jnz	short #loop
	and	eax,0ffH
	ret

po_hashfunc endp


;*****************************************************************************
;* void poco_stuff_bytes(void *mem, UBYTE value, int count)
;*****************************************************************************

poco_stuff_bytes proc	  near

	mov	edx,edi
	mov	edi,[esp+4]
	mov	eax,[esp+8]
	mov	ah,al
	mov	ecx,[esp+12]
	shr	ecx,1
	rep stosw
	adc	ecx,0
	rep stosb
#done:
	mov	edi,edx
	ret

poco_stuff_bytes endp


;*****************************************************************************
;* void poco_zero_bytes(void *mem, int count)	     - stack parms
;* void __poco_zero_bytes__(void *mem, int count)    - reg parms
;*****************************************************************************

poco_zero_bytes proc near

	push	edi
	mov	edi,[esp+8]
	mov	ecx,[esp+12]
	mov	edx,ecx
	and	edx,3
	shr	ecx,2
	xor	eax,eax
	rep stosd
	mov	ecx,edx
	rep stosb
	pop	edi
	ret

poco_zero_bytes endp

__poco_zero_bytes__ proc near

	push	edi
	mov	edx,ecx
	and	edx,3
	shr	ecx,2
	xor	eax,eax
	rep stosd
	mov	ecx,edx
	rep stosb
	pop	edi
	ret

__poco_zero_bytes__ endp


;*****************************************************************************
;* void poco_copy_bytes(void *source, void *dest, int count)	 - stack parms
;* void __poco_copy_bytes__(void *source, void *dest, int count) - reg parms
;*****************************************************************************

poco_copy_bytes proc near

	push	ebp
	mov	ebp,esp
	push	esi
	push	edi
	mov	esi,[ebp+8]
	mov	edi,[ebp+12]
	mov	ecx,[ebp+16]
	mov	edx,ecx
	and	edx,3
	shr	ecx,2
	rep movsd
	mov	ecx,edx
	rep movsb
	pop	edi
	pop	esi
	leave
	ret

poco_copy_bytes endp

__poco_copy_bytes__ proc     near

	push	esi
	mov	eax,edi
	mov	edx,ecx
	and	edx,3
	shr	ecx,2
	rep movsd
	mov	ecx,edx
	rep movsb
	mov	edi,eax
	pop	esi
	ret

__poco_copy_bytes__ endp

;*****************************************************************************
;* Boolean po_eqstrcmp(char *s1, char *s2)
;*  returns 0 if strings are equal, else non-zero.  (just like strcmp except
;*  it checks only equality, not less/greater than).
;*  (This one I'm actually proud of; it's semi-tricky).
;*****************************************************************************

po_eqstrcmp proc     near

	xor	eax,eax 		 ; Must start with high-order bits in
	mov	ecx,[esp+4]		 ; eax clean for return value.	Load
	mov	edx,[esp+8]		 ; pointers to the two strings.
#loop:
	mov	ah,[ecx]		 ; Load next s1 and next s2 bytes. Check
	mov	al,[edx]		 ; s1 for nullterm, if true, we're done.
	test	ah,ah			 ; (If s2 was also null, eax will be 0;
	jz	short #return		 ; the strings are equal in this case).
	test	al,al			 ; Check s2 for nullterm, if true, we're
	jz	short #return		 ; done, but since we know s1 (in ah)
	inc	ecx			 ; is not zero, we'll be indicating
	inc	edx			 ; strings are not equal.  If not at
	cmp	ah,al			 ; end of either string, incr ptrs, and
	je	short #loop		 ; compare the bytes.  If equal, loop,
#return:				 ; else return the (non-zero) value
	ret				 ; in eax.

po_eqstrcmp endp

;*****************************************************************************
;* char *po_chop_csym(char *line, char *word, int maxlen, char **wordnext)
;*   Note:  this function is #pragma'tized to take parms in registers:
;*	esi = line
;*	edi = word
;*	ecx = maxlen
;*	edx = wordnext
;*	return value in esi
;*****************************************************************************

po_chop_csym proc near

	push	ebp
	mov	ebp,esp
	push	esi
	push	edi

	mov	esi,[ebp+8]
	mov	edi,[ebp+12]
	mov	ecx,[ebp+16]
	mov	edx,[ebp+20]
	call	__po_chop_csym__

	mov	eax,esi
	pop	edi
	pop	esi
	leave
	ret

po_chop_csym endp


__po_chop_csym__ proc near

	push	edi
	xor	eax,eax
#loop:
	lodsb
	stosb
	test	byte ptr[eax+istab],ISCSYM
	loopnz	#loop
	jz	#return
#eatem:
	lodsb
	test	byte ptr[eax+istab],ISCSYM
	jnz	#eatem
#return:
	dec	esi
	dec	edi
	mov	[edx],edi
	pop	edi
	ret

__po_chop_csym__ endp

_CODE	ends
	end

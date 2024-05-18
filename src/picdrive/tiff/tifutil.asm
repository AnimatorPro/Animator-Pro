; Set your editor tabsize to 8 for this file!
;****************************************************************************
; TIFUTIL.ASM - TIFF driver utility routines.
;		Little things that are faster/easier to do in assembler.
;****************************************************************************

_TEXT	segment dword public use32 'CODE'
	assume	cs:_TEXT,DS:_TEXT

	public	swapw
	public	swapd
	public	unpack_samples
	public	xlatebuffer
	public	xlate2rgb

;****************************************************************************
; swapw - Swap the upper/lower bytes of a word in place.
;	void swapw(void *pointer_to_word_to_swap);
;****************************************************************************

swapw	proc	near
	mov	eax,[esp+4]
	mov	cx,word ptr[eax]
	xchg	ch,cl
	mov	word ptr[eax],cx
	ret
swapw	endp

;****************************************************************************
; swapd - Swap the upper/lower bytes and words of a dword in place.
;	void swapd(void *pointer_to_dword_to_swap);
;****************************************************************************

swapd	proc	near
	mov	eax,[esp+4]
	mov	ecx,dword ptr[eax]
	xchg	ch,cl
	rol	ecx,16
	xchg	ch,cl
	mov	dword ptr[eax],ecx
	ret
swapd	endp

;****************************************************************************
; unpack_samples - Unpack sample bits into full bytes.
;	void unpack_sample(char *source, char *dest, int count, int spb);
;****************************************************************************

unpack_samples	proc	near

	push	ebp
	mov	ebp,esp
	pushad
	mov	esi,[ebp+8]		; load input pointer
	mov	edi,[ebp+12]		; load output pointer
	mov	ecx,[ebp+16]		; load line length counter
	mov	edx,[ebp+20]		; load bits-per-sample
	mov	bh,1			; prime input bit counter
nextbyte:
	mov	al,0			; empty byte
	mov	ah,dl			; prime samples-per-bit loop count
nextbit:
	dec	bh			; out of input bits in current byte?
	jnz	short getbit		; nope, go do process the bit
	mov	bh,8			; yep, re-prime the bit counter
	mov	bl,byte ptr[esi]	; load a new input byte
	inc	esi			; increment input pointer
getbit:
	shl	al,1			; add room for bit to output byte
	shl	bl,1			; check next bit of input byte
	jnc	short offbit		; if bit not set on input, skip
	or	al,1			; else set bit on in output
offbit:
	dec	ah			; decrement bits-per-sample counter
	jnz	short nextbit		; if not zero, loop for next bit
	stosb				; byte is built, store it
	loop	nextbyte		; loop until width counter is zero

	popad
	leave
	ret

unpack_samples endp

;****************************************************************************
; xlatebuffer - Translate bytes from input to output buffer.
;
;	The input and output buffers can be the same (translate in place).
;
;	The translation table should be 256 bytes long (more to the point,
;	it should be as long as the highest value found in the input.)
;
;      void xlatebuffer(char *xltab, char *input, char *output, int count);
;
;	Interesting side effects dept:	This routine can be used to reverse
;	a fixed-length sequence of bytes.  Consider the following:
;	    char reversit[] = {5,4,3,2,1,0};
;	    char input[]    = "abcdef";
;	    char output[7];
;	    xlatebuffer(input, reversit, output, 6);
;	Of course, it's only realy efficient for larger sequences.
;****************************************************************************

xlatebuffer	proc	near

	push	ebp
	mov	ebp,esp
	push	ebx
	push	esi
	push	edi
	cld

	mov	ebx,[ebp+8]		; ptr to xtlate table
	mov	esi,[ebp+12]		; ptr to input bytes
	mov	edi,[ebp+16]		; ptr to output bytes
	mov	ecx,[ebp+20]		; length of input

mainloop:
	lodsb				; gotta admit, there's a few slick
	xlatb				; things about the 80x86 family.
	stosb				; (still not as good as the single-
	loop	mainloop		; instr block xlate on IBM 370 tho!)

	pop	edi
	pop	esi
	pop	ebx
	leave
	ret

xlatebuffer	endp

;*****************************************************************************
;* xlate2rgb(Rgb3 *ctab, char *source, char *dest, int width);
;*****************************************************************************

xlate2rgb proc	near

	push	ebp
	mov	ebp,esp
	push	ebx
	push	esi
	push	edi

	mov	ebx,[ebp+8]		; load color map pointer
	mov	edx,[ebp+12]		; load source pointer
	mov	edi,[ebp+16]		; load dest pointer
	mov	ecx,[ebp+20]		; load loop counter

rgbloop:
	movzx	eax,byte ptr [edx]
	inc	edx
	imul	eax,3
	lea	esi,[ebx+eax]
	movsw
	movsb
	loop	rgbloop

	pop	edi
	pop	esi
	pop	ebx
	leave
	ret
xlate2rgb endp

_TEXT	ends
	end

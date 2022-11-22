	include stdmacro.i

_TEXT	segment

	public pj_xlate

;*****************************************************************************
;* void pj_xlate(UBYTE *table, UBYTE *buf, unsigned count)
;*	table -> 256 byte translation table
;*	buf -> area of count bytes to translate
;*****************************************************************************

pj_xlate proc near

	Entry
	Args	#table,#buf,#count
	Save	ebx,edi

	mov	ebx,#table		; load table pointer
	mov	edi,#buf		; load buffer pointer
	mov	ecx,#count		; load count
	shr	ecx,1			; adjust to word count, set carry flag
xloop:

	mov	ax,[edi]		; load a word
	xlatb				; translate low byte
	xchg	ah,al			; swap bytes
	xlatb				; translate high byte
	xchg	ah,al			; swap bytes back
	stosw				; store word
	dec	ecx			; count the word we just did
	jnz	short xloop		; if more words, loop to do them.
	jnc	short alldone		; if SHR ECX,1 set carry above, we
	mov	al,[edi]		; have one more byte, load it,
	xlatb				; translate it,
	stosb				; and store it.

alldone:

	Restore ebx,edi
	Exit

pj_xlate endp

_TEXT	ends
	end

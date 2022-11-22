
	include stdmacro.i

_TEXT	segment
	public pj_copy_words

;*****************************************************************************
;* void pj_copy_words(void *src, void *dst, unsigned count)
;*****************************************************************************

pj_copy_words proc near

	Entry
	Args	#src,#dst,#count

	mov	eax,esi 		; save esi
	mov	edx,edi 		; save edi

	mov	esi,#src		; load source pointer
	mov	edi,#dst		; load destination pointer
	mov	ecx,#count		; load copy count
	shr	ecx,1			; adjust to dword count
	rep movsd			; move 'em
	adc	ecx,ecx 		; do last word (if any)
	rep movsw			; move it

	mov	edi,edx 		; restore edi
	mov	esi,eax 		; restore esi
	Exit

pj_copy_words endp

_TEXT	ends
	end

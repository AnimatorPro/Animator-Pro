
	include stdmacro.i

_TEXT	segment

	public pj_copy_bytes
	public pj_copy_structure

;*****************************************************************************
;* void pj_copy_bytes(void *src, void *dst, unsigned count)
;* void pj_copy_structure(void *src, void *dst, unsigned count)
;*
;*  (note: pj_copy_structure() used to assume an even copy count, but this
;*  is no longer a constraint, it'll handle odd sizes too.)
;*****************************************************************************

pj_copy_bytes proc near
pj_copy_structure proc near

	Entry
	Args	#src,#dst,#count

	mov	eax,esi 		; save esi
	mov	edx,edi 		; save edi

	mov	esi,#src		; load source pointer
	mov	edi,#dst		; load destination pointer
	mov	ecx,#count		; load copy count
	shr	ecx,1			; adjust to word count
	rep movsw			; move 'em
	adc	ecx,ecx 		; set up for last byte (if any)
	rep movsb			; move it

	mov	edi,edx 		; restore edi
	mov	esi,eax 		; restore esi
	Exit

pj_copy_structure endp
pj_copy_bytes endp

_TEXT	ends
	end

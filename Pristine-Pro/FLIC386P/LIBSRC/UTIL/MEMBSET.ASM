
	include stdmacro.i

_TEXT	segment

	public pj_stuff_bytes

;*****************************************************************************
;* void pj_stuff_bytes(UBYTE data, UBYTE *buf, unsigned count)
;*****************************************************************************

pj_stuff_bytes proc near

	Entry
	Args	#data,#buf,#count

	mov	edx,edi 		; save edi in edx
	mov	eax,#data		; load value
	mov	edi,#buf		; load destination pointer
	mov	ecx,#count		; load count
	mov	ah,al			; replicate byte
	shr	ecx,1			; adjust to word count
	rep stosw			; store it
	adc	ecx,ecx 		; add in extra byte (if any)
	rep stosb			; store it
	mov	edi,edx 		; restore edi

	Exit

pj_stuff_bytes	   endp

_TEXT	ends
	end

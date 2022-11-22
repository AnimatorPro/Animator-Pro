	include stdmacro.i

_TEXT	segment

	public pj_stuff_words

;*****************************************************************************
;* void pj_stuff_words(USHORT data, USHORT *buf, unsigned count)
;*****************************************************************************

pj_stuff_words proc near

	Entry
	Args	#data,#buf,#count

	mov	edx,edi 		; save edi in edx
	mov	edi,[esp+16]		; load destination pointer
	mov	eax,[esp+12]		; load value
	mov	ecx,[esp+20]		; load count
	rep stosw			; store it
	mov	edi,edx 		; restore edi

	Exit

pj_stuff_words endp

_TEXT	ends
	end

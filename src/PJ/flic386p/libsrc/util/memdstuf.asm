	include stdmacro.i

_TEXT	segment

	public pj_stuff_pointers
	public pj_stuff_dwords

;*****************************************************************************
;* void pj_stuff_dwords(ULONG data, ULONG *buf, unsigned count)
;* void pj_stuff_pointers(void *data, void **buf, unsigned count)
;*****************************************************************************

pj_stuff_pointers proc near
pj_stuff_dwords proc near

	Entry
	Args	#data,#buf,#count

	mov	edx,edi 		; save edi in edx
	mov	edi,[esp+16]		; load destination pointer
	mov	eax,[esp+12]		; load value
	mov	ecx,[esp+20]		; load count
	rep stosd			; store it
	mov	edi,edx 		; restore edi

	Exit

pj_stuff_dwords endp
pj_stuff_pointers	endp

_TEXT	ends
	end

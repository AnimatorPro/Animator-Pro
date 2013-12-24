
	include stdmacro.i

_TEXT	segment
	public pj_copy_dwords

;*****************************************************************************
;* void pj_copy_dwords(void *src, void *dst, unsigned count)
;*****************************************************************************

pj_copy_dwords proc near

	Entry
	Args	#src,#dst,#count

	mov	eax,esi 		; save esi
	mov	edx,edi 		; save edi

	mov	esi,#src		; load source pointer
	mov	edi,#dst		; load destination pointer
	mov	ecx,#count		; load copy count
	rep movsd			; move it

	mov	edi,edx 		; restore edi
	mov	esi,eax 		; restore esi
	Exit

pj_copy_dwords endp

_TEXT	ends
	end

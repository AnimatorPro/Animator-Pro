;*****************************************************************************
;* WCRALLOC.ASM - Allocate/Free DOS real (below 1mb line) memory (Watcom DOS4G).
;*
;*  We use DPMI calls to alloc and free memory from below the 1mb line.
;*
;*****************************************************************************

	include stdmacro.i
	include dosmem.i

Err_no_memory = -2

_TEXT	segment

	public	pj_dosmem_alloc
	public	pj_dosmem_free

;*****************************************************************************
;* Errcode pj_dosmem_alloc(size_t bytes, struct dosmem_block *block);
;*  This allocates a memory block of the specified size below the 1mb line.
;*  If it succeeds, it return Success and the structure at *block holds
;*  the selector:off32 and dos segment number of the realmode memory block.
;*  If it fails, it returns Err_no_memory.
;*****************************************************************************

pj_dosmem_alloc proc

	Entry
	Args	#bytes,#block
	Save	ebx,esi,edi

	mov	ebx,#bytes		; load requested size,
	add	ebx,15			; round up to next 16-byte value,
	shr	ebx,4			; adjust to paragraph count.
	mov	eax,0100h		; DPMI alloc-dos-memory function,
	int 31h 			; do it.  if CARRY is clear, go fill
	jnc	short #good		; in return value structure, else
	mov	eax,Err_no_memory	; return Err_no_memory.
	jmp	short #return
#good:
	mov	ebx,#block		; DPMI returns the dos segment number
	mov	[ebx+6],ax		; in AX, just store it.  We construct
	xor	eax,eax 		; the 16:32 pointer from the selector
	mov	[ebx],eax		; DPMI returned in DX and a 0 offset
	mov	[ebx+4],dx		; (which is also our Success retval).
#return:
	Restore ebx,esi,edi
	Exit

pj_dosmem_alloc endp


;*****************************************************************************
;* void pj_dosmem_free(struct dosmem_block *block)
;*   this accepts a pointer to a dosmem_block structure which describes
;*   a dos memory block allocated by pj_dosmem_alloc, and frees the block.
;*****************************************************************************

pj_dosmem_free proc

	Entry
	Args	#block
	Save	ebx,esi,edi

	mov	edx,#block		; get pointer to 16:32 pointer, if
	jz	short #punt		; it's NULL, just punt, else get the
	movzx	edx,wptr [edx+4]	; segment selector part of the 16:32
	test	edx,edx 		; pointer.  if selector is zero,
	jz	short #punt		; just punt.
	mov	ax,0101h		; DPMI free DOS memory function,
	int 31h 			; do it.
	mov	edx,#block
	xor	eax,eax
	mov	[edx],eax
	mov	[edx+4],eax
#punt:
	Restore ebx,esi,edi
	Exit

pj_dosmem_free endp

_TEXT	ends
	end

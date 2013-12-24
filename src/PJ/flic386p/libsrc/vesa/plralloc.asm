;*****************************************************************************
;* PLRALLOC.ASM - Allocate/Free DOS real (below 1mb line) memory (Phar Lap).
;*
;*  Phar Lap has special functions for allocating memory below the 1mb line.
;*  It (like native DOS) deals with things in terms of segments and
;*  paragraphs.  Our caller deals with far32 pointers and bytes, so the
;*  routines below translate the values appropriately.
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

pj_dosmem_alloc proc near

	Entry
	Args	#bytes,#block
	Save	ebx,esi,edi

	mov	ebx,#bytes		; load requested size,
	add	ebx,15			; round up to next 16-byte value,
	shr	ebx,4			; adjust to paragraph count.
	mov	eax,25c0h		; pharlap alloc-dos-memory function,
	int 21h 			; do it.  if CARRY set, go handle
	jnc	short #good		; error, else move returned DOS
	mov	eax,Err_no_memory
	jmp	short #return
#good:
	mov	ebx,#block		; PLS returns the dos segment number
	mov	[ebx+6],ax		; in AX, just store it.  We construct
	movzx	eax,ax			; the 16:32 pointer by cleaning up the
	shl	eax,4			; high bits of the dos segment number,
	mov	[ebx],eax		; shifting up to create a 32-bit offset,
	mov	[ebx+4],gs		; and using the global below-1mb-line
	xor	eax,eax 		; selector from GS.  Return Success.
#return:
	Restore ebx,esi,edi
	Exit

pj_dosmem_alloc endp


;*****************************************************************************
;* void pj_dosmem_free(struct dosmem_block *block)
;*   this accepts a pointer to a dosmem_block structure which describes
;*   a dos memory block allocated by pj_dosmem_alloc, and frees the block.
;*****************************************************************************

pj_dosmem_free proc near

	Entry
	Args	#block
	Save	ebx,esi,edi

	mov	ecx,#block		; load pointer to dosmem_block struct.
	test	ecx,ecx 		; if pointer is NULL, just punt,
	jz	short #punt		; else load the dos segment number
	movzx	ecx,wptr [ecx+6]	; from the structure, and if it's
	test	ecx,ecx 		; NULL, just punt, else pass it
	jz	short #punt		; to the pharlap free DOS memory
	mov	ax,25C1h		; function.
	int 21h 			; do it.
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

;*****************************************************************************
;* CBRALLOC.ASM - Allocate/Free DOS real (below 1mb line) memory (CodeBldr).
;*
;*  Codebuilder allocates memory from below the 1mb line if the high bit of
;*  the function code in eax is cleared.  It uses the normal DOS function
;*  number, but deals with flat pointers and byte counts, instead of segments
;*  and paragraphs, like DOS does.
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
	Save	ebx,esi,edi		; be safe; not sure DOS touches these.

	mov	ebx,#bytes		; load requested size, and func code for
	mov	eax,00004800h		; CodeBuilder alloc-dos-memory function.
	int 21h 			; do it.  if CARRY is clear, go fill
	jnc	short #good		; in return value structure, else
	mov	eax,Err_no_memory	; return Err_no_memory.
	jmp	short #return
#good:
	mov	ebx,#block		; CB/DOS returns the flat address in
	mov	[ebx],eax		; in EAX, just store it.  The selector
	mov	[ebx+4],gs		; part comes from the global below-1mb
	shr	eax,4			; selector from GS.  The dos segment
	mov	[ebx+6],ax		; number is calculated by shifting the
	xor	eax,eax 		; flat address.  Return Success.
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

	mov	edx,#block		; load pointer to block.  if pointer
	test	edx,edx 		; is NULL just punt, else get the
	jz	short #punt		; 32-bit offset part of the far32
	mov	edx,[edx]		; pointer it points to, and check to
	test	edx,edx 		; see if it's NULL.  If so, just punt,
	jz	short #punt		; else pass it to the CodeBuilder
	mov	ax,00004900h		; free DOS memory function.
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

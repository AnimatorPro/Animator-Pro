	include stdmacro.i

dpmi_memory_info struc
freebytes	 dd ?		; 00h	Largest available free block in bytes
freeunlocked	 dd ?		; 04h	Maximum unlocked page allocation
freelocked	 dd ?		; 08h	Maximum locked page allocation
addrspace	 dd ?		; 0Ch	Linear addr space size in pages
totalunlocked	 dd ?		; 10h	Total number of unlocked pages
freepages	 dd ?		; 14h	Number of free pages
physpages	 dd ?		; 18h	Total number of physical pages
addrfree	 dd ?		; 1Ch	Free linear address space in pages
pagefilesize	 dd ?		; 20h	Size of paging file/partition in pages
reserved	 dd 3 dup (?)	; 24h-2Fh	     Reserved
bytesperpage	 dd ?
dpmimachine	 db ?
dpmiflags	 db ?
dpmiminor	 db ?
dpmimajor	 db ?
dpmi_memory_info ends

_TEXT	segment

	public	pj_dpmi_inquire_version
	public	pj_dpmi_inquire_memory

;*****************************************************************************
;* ULONG pj_dpmi_inquire_version(void);
;*  returns VVvvFFPP
;*	    | | | |___ Processor type  (2 = 286, 3 = 386, 4= 486)
;*	    | | |_____ Flag bits       (see below)
;*	    | |_______ DPMI minor version number
;*	    |_________ DPMI major version number
;*
;* Returns zero if DPMI is not present.
;*
;* Flag bits:
;*   Bit 0 = 1 if running under an 80386 DPMI implementation.
;*   Bit 1 = 1 if processor is returned to real mode for reflected
;*	       interrupts (as opposed to Virtual 8086 mode).
;*   Bit 2 = 1 if virtual memory is supported.
;*   Bit 3     reserved and undefined.
;*   All other bits are zero and reserved for later use.
;*
;*****************************************************************************

pj_dpmi_inquire_version proc

	Entry
	Save	ebx,esi,edi,es

	mov	eax,1687h
	int 2Fh
	test	ax,ax
	jnz	short #error

	mov	eax,0400h
	int 31h
	shl	eax,16
	mov	ah,bl
	mov	al,cl
	jmp	short #return
#error:
	xor	eax,eax
#return:
	Restore ebx,esi,edi,es
	Exit

pj_dpmi_inquire_version endp


;*****************************************************************************
;* Boolean pj_dpmi_inquire_memory(DPMI_memory_info *pinfo);
;*****************************************************************************

pj_dpmi_inquire_memory proc

	Entry
	Args	#pinfo
	Save	ebx,edi

	mov	edi,#pinfo
	call	pj_dpmi_inquire_version
	mov	dptr [edi].dpmimachine,eax
	test	eax,eax
	jz	short error

	mov	eax,0604h
	int 31h
	jc	short error
	shl	ebx,16
	and	ecx,0000FFFFh
	or	ecx,ebx
	mov	[edi].bytesperpage,ecx

	mov	eax,0500h
	int 31h
	jc	short error
	mov	eax,1
	jmp	short return
error:
	xor	eax,eax
return:
	Restore ebx,edi
	Exit

pj_dpmi_inquire_memory endp


_TEXT	ends
	end

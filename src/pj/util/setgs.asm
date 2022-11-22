;*****************************************************************************
;* SETGS.ASM - Routine to set GS segreg to PHAR_REAL_SEG, the descriptor
;*	       that maps the first megabyte of physical memory.
;*
;* For the FlicLib code, it should be possible for the client to supply a
;* pj_set_gs() routine appropriate to DOS extenders other than Phar Lap.
;* All routines in FlicLib that need to access DOS memory (clock routines,
;* video drivers, etc), assume that GS will be holding the right segment
;* descriptor.	They all use this function to load that descriptor into GS.
;* Nowhere in the FlicLib code is the PHAR_REAL_SEG, VGA_SEG, or 0x34
;* constant coded.
;*
;* To replace this routine, make sure that your replacement:
;*  - loads the right descriptor into GS.
;*  - returns the same descriptor in eax.
;*  - can be called any number of times.
;* The latter item is important, because the MCGA driver will call this
;* routine (potentially multiple times) just to get the descriptor.
;*
;* Added 09/27/91: Routines for getting GS and DS segreg values.  If you
;*		   are working with a non-Phar Lap extender, code all 3 of
;*		   these as appropriate to your extender.
;*****************************************************************************

PHAR_REAL_SEG equ 34h			; Phar Lap descriptor for real memory

code	segment public dword use32 'CODE'
	assume cs:code

	public pj_set_gs
	public pj_get_gs
	public pj_get_ds

;*****************************************************************************
;* short pj_set_gs(void) - Set the GS reg, return its new value in eax.
;*****************************************************************************

pj_set_gs proc near
	mov	eax,PHAR_REAL_SEG	; set return value.
	mov	gs,ax			; move value to GS segreg.
	ret
pj_set_gs endp

;*****************************************************************************
;* short pj_get_gs(void) - Return the value currently in the GS segreg.
;*****************************************************************************

pj_get_gs proc near
	mov	ax,gs			; return value from GS segreg
	and	eax,0FFFFh
	ret
pj_get_gs endp

;*****************************************************************************
;* short pj_get_ds(void) - Return the value currently in the DS segreg.
;*****************************************************************************

pj_get_ds proc near
	mov	ax,ds			; return value from GS segreg
	and	eax,0FFFFh
	ret
pj_get_ds endp

code	ends
	end

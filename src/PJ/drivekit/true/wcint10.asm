;*****************************************************************************
;* WCINT10.ASM - Issue int10/getinfo to vesa bios (CodeBuilder).
;*
;*  This is not used in the loadable device driver, only in the watcom/dos4g
;*  version of the fliclib.
;*
;*  Two of the vesa functions we call via int 10, VBE_INFO and MODE_INFO,
;*  require a pointer to a DOS-area memory buffer to be passed in ES:DI.
;*  For DOS4G, we use the DPMI 0300h (issue realmode interupt) function.
;*
;*  The bulk of the vesa code uses an inline int 10 instruction to talk to
;*  the vesa bios.  This module is used only for the two INFO calls that
;*  require the buffer pointer; these occur only during mode switches and
;*  get_mode_info calls, and performance is not an issue in this routine.
;*****************************************************************************

	include stdmacro.i		; our standard macros

_DATA  segment

realregs struc			       ; offset
rr_edi	  dd   ?		       ;   00h
rr_esi	  dd   ?		       ;   04h
rr_ebp	  dd   ?		       ;   08h
rr_rsvd   dd   ?		       ;   0ch
rr_ebx	  dd   ?		       ;   10h
rr_edx	  dd   ?		       ;   14h
rr_ecx	  dd   ?		       ;   18h
rr_eax	  dd   ?		       ;   1ch
rr_flags  dw   ?		       ;   20h
rr_es	  dw   ?		       ;   22h
rr_ds	  dw   ?		       ;   24h
rr_fs	  dw   ?		       ;   26h
rr_gs	  dw   ?		       ;   28h
rr_ip	  dw   ?		       ;   2ah
rr_cs	  dw   ?		       ;   2ch
rr_sp	  dw   ?		       ;   2eh
rr_ss	  dw   ?		       ;   30h
realregs ends

realparms realregs <0>		       ; alloc struct, init to zero

_DATA  ends

_TEXT	segment

	public	_pj_vesa_int10

;*****************************************************************************
;* _pj_vesa_int10
;*
;*  Issue INT10 call to VESA BIOS, such that flat pointer in edi is seen by
;*  the real mode handler as a segmented pointer in ES:DI.  The value in edi
;*  will be a pointer to a buffer in real DOS memory.
;*
;*  Input:
;*	eax = VESA function number.
;*	edi = pointer (flat) to buffer in DOS memory.
;*  Output:
;*	values returned by VESA BIOS.
;*****************************************************************************

_pj_vesa_int10 proc near

	Entry
	Save   ebx,esi,edi

	mov    realparms.rr_eax,eax    ; save eax in realparms struct
	lea    eax,realparms	       ; load pointer to realparms

	mov    [eax].rr_ebx,ebx        ; save other regs in realparms struct
	mov    [eax].rr_ecx,ecx
	mov    [eax].rr_edx,edx
	mov    [eax].rr_esi,esi

	mov    edx,edi		       ; move pointer to work reg
	shr    edx,4		       ; convert flat to segmented
	mov    [eax].rr_ds,dx	       ; save segment part of pointer
	mov    [eax].rr_es,dx	       ; in realmode DS and ES regs.

	and    edi,0000000Fh	       ; mask segment out of pointer
	mov    realparms.rr_edi,edi    ; save offset part of pointer

	mov    edi,eax
	xor    ecx,ecx		       ; copy 0 bytes to real stack
	mov    ebx,0010h	       ; issue realmode int 10h
	mov    eax,0300h	       ; DPMI function number
	int 31h 		       ; do it
	mov    eax,realparms.rr_eax    ; load retval

	Restore ebx,esi,edi
	Exit

_pj_vesa_int10 endp

_TEXT	ends
	end

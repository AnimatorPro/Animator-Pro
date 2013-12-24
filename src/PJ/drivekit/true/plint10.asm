;*****************************************************************************
;* PLINT10.ASM - Issue int10/getinfo to vesa bios (Pharlap).
;*
;*  Two of the vesa functions we call via int 10, VBE_INFO and MODE_INFO,
;*  require a pointer to a DOS-area memory buffer to be passed in ES:DI.
;*  For Phar Lap, there is a special function call which invokes int 10
;*  and loads the segment registers with values from a parmblk structure
;*  before giving control to the real mode interupt handler.
;*
;*  The bulk of the vesa code uses an inline int 10 instruction to talk to
;*  the vesa bios.  This module is used only for the two INFO calls that
;*  require the buffer pointer; these occur only during mode switches and
;*  get_mode_info calls, and performance is not an issue in this routine.
;*****************************************************************************

	include stdmacro.i

;-----------------------------------------------------------------------------
; Phar Lap structure for issuing Real Mode INT...
;-----------------------------------------------------------------------------

parblk	struc
pbint	dw	?			; Interrupt to issue
pbds	dw	?			; DS value
pbes	dw	?			; ES value
pbfs	dw	?			; FS value
pbgs	dw	?			; GS value
pbeax	dd	?			; eax value
pbedx	dd	?			; edx value
parblk	ends

_BSS	segment
pb	parblk	<>			; general int10 interface structure
_BSS	ends

_TEXT	segment

	public	_pj_vesa_int10

;*****************************************************************************
;* _pj_vesa_int10
;*  Issue INT10 call to VESA BIOS such that VESA sees a dos memory buffer
;*  pointer in ES:DI.  The caller passes the dos segment number in edi,
;*  and we put it in ES for the call, and zero out the offset (DI) part.
;*
;*  Input:
;*	eax	= VESA function number.
;*	edi	= dos segment number (the ES part of ES:0000 far pointer)
;*   other regs = as required for the vesa bios call.
;*  Output:
;*	values returned by VESA BIOS.
;*****************************************************************************

_pj_vesa_int10 proc near

	mov	pb.pbint,10h		; Store interupt number.
	mov	pb.pbeax,eax		; Store caller's eax.
	mov	pb.pbedx,edx		; Store caller's edx.
	mov	pb.pbes,di		; Store dos buffer segment number.
	xor	edi,edi 		; Offset part of address is zero.

	mov	ax,2511h		; Phar Lap function for arbitrary INT.
	lea	edx,pb			; Pointer to parameter block
	int 21h 			; Issue the interrupt.

	cld				; Let's be paranoid.
	ret

_pj_vesa_int10 endp

_TEXT	ends
	end

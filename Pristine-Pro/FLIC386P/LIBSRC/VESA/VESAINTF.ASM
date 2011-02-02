;*****************************************************************************
;* VESAINTF.ASM - VESA BIOS interfaces.
;*
;*  NOTES:
;*		This module contains routines to detect the presence of the
;*		VESA BIOS, query the available modes, enter and leave those
;*		modes, and perform bank switches via VESA BIOS calls.
;*
;*		If you're looking at this code for the first time, be sure to
;*		see the comments in DRVCOMN.I concerning the wcontrol structure.
;*
;*  MAINTENANCE:
;*    ??/??/91	Jim Byers
;*		Original version (hardware.asm).
;*    03/27/91	Ian Lepore
;*		Overall driver was rewritten.  The code in this module was
;*		mostly copied from the original driver's hardware.asm module.
;*		All the output routines were removed to their own modules,
;*		then code was added herein to build the ytable during mode
;*		initialization.  Other small changes were made to glue the
;*		original hardware.asm code to changes in device.c.
;*    04/25/91	Ian Lepore
;*		Changed the pj_vdrv_set_colors routine (see comments there),
;*		and added pj_vdrv_uncc64 and pj_vdrv_uncc256.
;*    05/28/91	Ian
;*		Split out non-VESA code (set colors, wait_sync) to DRVCOMN.ASM.
;*    06/03/91	Ian
;*		Heavily modified, especially the pj_vesa_detect routine, for
;*		use in the new integrated autodetect supervga driver.  These
;*		changes mostly consisted of having pj_vesa_detect fill in the
;*		new SMInfo structure array instead of the old vesamodes array,
;*		and a conversion to parms-in-regs as required by the svga
;*		init code that calls pj_vesa_detect.
;*    07/30/91	Ian
;*		This module is now shared by the vesa driver and the svga
;*		autodetect driver. All global names now start with pj_ for
;*		compatibility with the FLILIB stuff.
;*    12/06/91	Ian
;*		Did some tweaks for PharLap/IntelCB compatibility.  Where we
;*		used to directly call a pharlap function to allocate a buffer
;*		in the DOS memory region, we now call pj_dosmem_alloc() to
;*		get it.  Where we used to make inline calls to pharlap's
;*		issue-arbitrary-int-with-regs function, we now call
;*		pj_vesa_int10().  These new functions live in separate
;*		modules, allowing them to be easily replaced with code
;*		appropriate to any given DOS extender.
;*****************************************************************************

;******************************************************************************
;*									      *
;*		   Copyright (C) 1991 by Autodesk, Inc. 		      *
;*									      *
;*	Permission to use, copy, modify, and distribute this software and     *
;*	its documentation for the purpose of creating applications for	      *
;*	Autodesk Animator, is hereby granted in accordance with the terms     *
;*	of the License Agreement accompanying this product.		      *
;*									      *
;*	Autodesk makes no warrantees, express or implied, as to the	      *
;*	correctness of this code or any derivative works which incorporate    *
;*	it.  Autodesk provides the code on an ''as-is'' basis and             *
;*	explicitly disclaims any liability, express or implied, for	      *
;*	errors, omissions, and other problems in the code, including	      *
;*	consequential and incidental damages.				      *
;*									      *
;*	Use, duplication, or disclosure by the U.S.  Government is	      *
;*	subject to restrictions set forth in FAR 52.227-19 (Commercial	      *
;*	Computer Software - Restricted Rights) and DFAR 252.227-7013 (c)      *
;*	(1) (ii) (Rights in Technical Data and Computer Software, as	      *
;*	applicable.							      *
;*									      *
;******************************************************************************

	include stdmacro.i
	include drvcomn.i
	include dosmem.i
	include errcodes.i
	include raster.i

;*****************************************************************************
;* Equates and data structure definitions...
;*****************************************************************************

MAXVMODES	equ	100		; size of our local modes array

VIDEO_INT	equ	10h		; DOS BIOS video interrupt

VESA_VBE_INFO	equ	4f00h		; get VESA hardware information
VESA_MODE_INFO	equ	4f01h		; get video mode information
VESA_SET_MODE	equ	4f02h		; set video mode
VESA_GET_MODE	equ	4f03h		; get video mode
VESA_SAV_RSTR	equ	4f04h
VESA_WIN_FUNC	equ	4f05h		; window functions

VESA_SAVBUFSIZE equ	0
VESA_SAVE	equ	1
VESA_RESTORE	equ	2
VESA_STATEMASK	equ	000fh

CAN_READ	equ	3
CAN_WRITE	equ	5
CAN_READ_WRITE	equ	7

VSUCCESSFUL	equ	004fh		; VESA function supported & successful

SUCCESS 	equ	0

ATTR_MODE_SUPPORTED equ     1
ATTR_EXTENDED_INFO  equ     2
ATTR_IS_GRAPHICS    equ     10h

;-----------------------------------------------------------------------------
; VESA info structure...
;  As defined by the vesa standard.
;-----------------------------------------------------------------------------

VBEInfo        struc
vi_Signature	db  4 dup (?)
vi_Version	dw  ?
vi_OEMStringPtr dd  ?
vi_Cap		db  4 dup (?)
vi_ModesPtr	dd  ?
vi_TotalMem	dw  ?
vi_filler	db  236 dup (?)
VBEInfo        ends

;-----------------------------------------------------------------------------
; VESA mode structure...
;  As defined by the vesa standard.
;-----------------------------------------------------------------------------

ModeInfo	struc
mi_modeattr	dw  ?			; Mandatory infomation...
mi_winAattr	db  ?
mi_winBattr	db  ?
mi_wingran	dw  ?			; window granularity (in KBytes)
mi_winsize	dw  ?			; window size (in Kbytes)
mi_winAseg	dw  ?			; window A start segment
mi_winBseg	dw  ?			; window B start segment
mi_winFuncPtr	dd  ?
mi_pitch	dw  ?			; bytes per scan line
mi_xsize	dw  ?			; Extended mode-info....
mi_ysize	dw  ?
mi_xcharsize	db  ?
mi_ycharsize	db  ?
mi_numplanes	db  ?
mi_bitsperpixel db  ?
mi_numbanks	db  ?
mi_memmodel	db  ?
mi_banksize	db  ?
mi_numimages	db  ?
mi_reserved	db  ?
mi_filler	db  242 dup (?)
ModeInfo	ends

;*****************************************************************************
;* data...
;*****************************************************************************


_BSS	segment

	align 2

bios_info  VBEInfo   <> 		; VESA BIOS information block
mode_info  ModeInfo  <> 		; VESA Mode Information Block
mode_array dw	     MAXVMODES dup (?)	; VESA Video Modes array

_BSS	ends

_DATA	segment

info_buffer  dosmem_block  <0>		; allocated realmode buffer
state_buffer dosmem_block  <0>		; allocated realmode save/restore buf

_DATA	ends

;*****************************************************************************
;* code...
;*****************************************************************************

_TEXT	segment

	public	_pj_vesa_setmode	; asm entry points
	public	_pj_vesa_clrmode

	public	pj_vesa_detect		; C entry points
	public	pj_vesa_setmode
	public	pj_vesa_clrmode
	public	pj_vesa_free_dosbuf
	public	pj_vesa_get_bios_info
	public	pj_vesa_get_mode_info

	extrn	pj_vesa_build_sminfo:near

	extrn	_pj_vesa_int10:near

;*****************************************************************************
;* set_write_bank
;*
;*  Entry:
;*	eax =  bank number
;*	edx -> pj_vdrv_wcontrol
;*  Exit:
;*	eax trashed, all others preserved.
;*****************************************************************************

	align 4
set_write_bank proc near

	push	ebx
	mov	[edx].wwrcurbank,eax	; save the new bank number as the
	test	[edx].wsamerw,1 	; current write bank.  if read & write
	jz	short #notsame		; windows are the same, save as current
	mov	[edx].wrdcurbank,eax	; read bank as well.
#notsame:
	mov	ebx,ecx
	mov	cl,[edx].wgranshift	; load shift count which turns a bank #
	shl	eax,cl			; into a window number, do the shift.
	mov	ecx,ebx
	mov	ebx,[edx].wwrwhich
	mov	edx,eax
	mov	eax,VESA_WIN_FUNC
	int	10h
	pop	ebx
	lea	edx,pj_vdrv_wcontrol		; restore edx for our caller.
	ret

set_write_bank endp

;*****************************************************************************
;* set_read_bank
;*  Entry:
;*	eax =  bank number
;*	edx -> pj_vdrv_wcontrol
;*  Exit:
;*	eax trashed, others preserved.
;*****************************************************************************

	align 4
set_read_bank proc near

	push	ebx
	mov	[edx].wrdcurbank,eax	; save the new bank number as the
	test	[edx].wsamerw,1 	; current read bank.  if read & read
	jz	short #notsame		; windows are the same, save as current
	mov	[edx].wwrcurbank,eax	; read bank as well.
#notsame:
	mov	ebx,ecx
	mov	cl,[edx].wgranshift	; load shift count which turns a bank #
	shl	eax,cl			; into a window number, do the shift.
	mov	ecx,ebx
	mov	ebx,[edx].wrdwhich
	mov	edx,eax
	mov	eax,VESA_WIN_FUNC
	int	10h
	pop	ebx
	lea	edx,pj_vdrv_wcontrol
	ret

set_read_bank endp

;*****************************************************************************
;* Errcode alloc_info_buffer(void)
;*****************************************************************************

alloc_info_buffer proc near

	Entry

	xor	eax,eax 		; assume success in case we have buffer.
	lea	ecx,info_buffer 	; load pointer to dosmem_block
	test	[ecx].dosseg,0FFFFh	; structure for info buffer, if
	jnz	short #return		; dosseg is non-zero, we have buffer.
	push	ecx			; otherwise, push pointer to structure
	push	256			; and size to allocate (256 bytes),
	call	pj_dosmem_alloc 	; and go get a buffer below 1mb line.
	add	esp,8			; clean up stack.
	jmp	short #return		; return status of buffer allocation.
#return:
	Exit

alloc_info_buffer endp

;*****************************************************************************
;* VBEInfo *pj_vesa_get_vbeinfo(void)
;*	returns a pointer to our local copy of the VBE info block.
;*****************************************************************************

pj_vesa_get_bios_info proc near

	lea	eax,bios_info
	ret

pj_vesa_get_bios_info endp

;*****************************************************************************
; VModeInfo *pj_vesa_get_mode_info(int modenumber) - called from C
;
;  This routine will query the VESA BIOS for information on the specified
;  mode, and will return a pointer to the resulting mode info structure. If
;  an error occurs during the query, a NULL pointer will be returned.
;*****************************************************************************

pj_vesa_get_mode_info proc near

	mov	ecx,[esp+4]		; load mode number

_pj_vesa_get_mode_info: 		; asm entry, mode already in ecx

	push	edi
	push	esi

	mov	di,info_buffer.dosseg	; load realmode buffer segment.
	mov	eax,VESA_MODE_INFO
	call	_pj_vesa_int10

	cmp	ax,VSUCCESSFUL		; did VESA_MODE_INFO call work?
	je	short #got_modeinf	; yep, continue processing.
	xor	eax,eax 		; nope, return NULL to caller.
	jmp	short #punt

#got_modeinf:

	; move the mode information into local 'mode_info' for easy access

	push	ds			; save ds.  load 16:32 pointer to
	lds	esi,info_buffer.far32	; dos realmode buffer into ds:esi.
	lea	edi,mode_info		; edi points to our near buffer;
	mov	eax,edi 		; save address for return value.
	mov	ecx,256/4		; move 256 bytes, using fast
	rep movsd			; dword moves.
	pop	ds			; restore ds.

#punt:
	pop	esi
	pop	edi
	ret

pj_vesa_get_mode_info	endp

;*****************************************************************************
;* _pj_vesa_clrmode - called from assembler
;*
;* Entry:
;*   esi - old_mode_data - the longword that setmode returned.
;*   edi - pointer to pj_vdrv_wcontrol
;* Exit:
;*   any regs may be trashed as desired.
;*****************************************************************************

_pj_vesa_clrmode proc near

	mov	eax,VESA_SET_MODE
	mov	ebx,esi
	int	10h
	ret

_pj_vesa_clrmode endp

;*****************************************************************************
;* _pj_vesa_setmode - called from assembler
;*
;* This routine sets a given video mode.  If the mode is a super VGA mode,
;* the wcontrol and ytable data structures are filled in based on information
;* provided by VESA about the mode.
;*
;* Entry:
;*   esi	- Points to SMInfo that describes mode to be set.
;*   edi	- Points to wcontrol.
;*   wcontrol	- wwidth, wheight, and wpitch have been set based on the values
;*		  in the SMInfo entry.	the setmode routine can override them
;*		  if necessary (only pitch might ever change, you would think).
;* Exit:
;*   eax	     - negative error code on failure, or any non-negative
;*		       longword on sucess.  the success longword will be
;*		       passed to the corresponding clrmode() routine on
;*		       device close.
;*   all other regs  - may be trashed with impunity.
;*   wcontrol	     - must be filled in as appropriate for the mode.
;*   ytable	     - must be filled in as appropriate for the mode.
;*****************************************************************************

_pj_vesa_setmode   proc    near

	mov	eax,VESA_GET_MODE	; get the current video mode
	int	10h			; for return value.
	movzx	ebp,bx			; save oldmode in ebp.

	mov	eax,VESA_SET_MODE
	movzx	ebx,[esi].smi_mode
	int	10h			; set the new mode
	cmp	eax,VSUCCESSFUL 	; check the return code
	jne	short #vbe_error	; if bad, go return failure status.

	mov	eax,VESA_GET_MODE	; get the new video mode
	int	10h			; to make extra sure the mode-set
	cmp	bx,[esi].smi_mode	; worked (vbe has been known to lie
	je	short #get_info 	; about the success of a mode-set!)
#vbe_error:
	mov	ebp,err_video_bios	; if mode-set failed indicate such,
	jmp	#punt			; and punt.

#get_info:
	movzx	ecx,[esi].smi_mode
	call	_pj_vesa_get_mode_info
	test	eax,eax
	jnz	short #got_info
	mov	ebp,err_no_vram
	jmp	#punt

#got_info:

	; if not VESA mode, don't compute VESA environment
	; this little section is weird:  I don't think it's needed
	; unless it's here to disallow some quirky video card.

	movzx	eax, mode_info.mi_modeattr
	and	eax, ATTR_EXTENDED_INFO+ATTR_IS_GRAPHICS
	cmp	eax, ATTR_EXTENDED_INFO+ATTR_IS_GRAPHICS
	jz	short #is_super_vga

	cmp	[esi].smi_mode,6Ah
	jb	#punt

#is_super_vga:

	lea	esi,mode_info

	; - check which windows can read, write, or both
	;   remember what their segments are

	mov	al,[esi].mi_winAattr	; get window A attributes,
	and	al,CAN_READ_WRITE	; mask to read/write status.
	mov	ah,[esi].mi_winBattr	; get window B attributes,
	and	ah,CAN_READ_WRITE	; mask to read/write status.

#a_readable?:

	test	al,CAN_READ		; can window A read?
	jz	short #a_writeable?	; nope, see if it's writeable.
	mov	[edi].wrdwhich,0	; yep, indicate A for reads.
	movzx	edx,[esi].mi_winAseg	; get A's segment,
	mov	[edi].wrdaddr,edx	; remember it as read address.

#a_writeable?:

	test	al,CAN_WRITE		; can window A write?
	jz	short #b_readable?	; nope, go check window B.
	mov	[edi].wwrwhich,0	; yep, indicate A for writes.
	movzx	edx,[esi].mi_winAseg	; get A's segment,
	mov	[edi].wwraddr,edx	; remember it as write address.

#b_readable?:

	test	ah,CAN_READ		; now it gets weird.  if B is readable
	jz	short #b_writeable?	; and A is writeable, we'll use A for
	test	al,CAN_WRITE		; writing (it's already set as such)
	jz	short #b_writeable?	; and B for reading.
	mov	[edi].wrdwhich,1	; indicate B for reads.
	movzx	edx, [esi].mi_winBseg	; get B's segment,
	mov	[edi].wrdaddr,edx	; remember it as read address.
	jmp	#got_windows		; all done.

#b_writeable?:

	test	ah,CAN_WRITE		; is window B writeable?
	jz	short #got_windows	; nope, we're stuck with window A.
	mov	[edi].wwrwhich,1	; yep, indicate B for writes.
	movzx	edx,[esi].mi_winBseg	; get B's segment,
	mov	[edi].wwraddr,edx	; remember it as write address.

#got_windows:

	mov	eax,[edi].wrdwhich	; load read window, compare to
	cmp	eax,[edi].wwrwhich	; write window; if equal, set flag
	sete	[edi].wsamerw		; showing read/write is same window.

	; - convert a real-mode segment values to protected-mode offsets

	shl	[edi].wrdaddr,4
	shl	[edi].wwraddr,4

	; - compute 'granshift', the number of left-shifts of the bank
	;   number that will produce the corresponding window number.

	xor	edx,edx
	movzx	eax,[esi].mi_winsize
	div	[esi].mi_wingran
	bsf	ebx,eax
	mov	[edi].wgranshift,bl

	; - compute 'bankshift', the number of right-shifts on the
	;   video address to obtain the bank.
	; - compute 'offsmask', this masks the video address
	;   leaving the offset from the start of the window.
	; - compute 'windwords', the number of dwords in a normal window.

	movzx	eax,[esi].mi_winsize
	bsf	ebx,eax
	add	bl,10
	mov	[edi].wbankshift,bl
	xor	eax,eax
	bts	eax,ebx
	dec	eax
	mov	[edi].woffsmask,eax
	inc	eax			; offset mask plus one is the number
	shr	eax,2			; of bytes in a window.  divide by 4
	mov	[edi].wwindwords,eax	; to get number of dwords, then save it.

	;   get the line-to-line pitch and save it in our pj_vdrv_wcontrol structure

	movzx	edx,[esi].mi_pitch
	mov	[edi].wpitch,edx

	; - compute the number of windows to flood a screen, and how many
	;   bytes to be filled in the last window.

	mov	eax,[edi].wheight	; height of screen times line pitch
	mul	dptr [edi].wpitch	; gives total bytes per screen. total
	mov	ecx,[edi].wwindwords	; bytes divided by bytes per window
	shl	ecx,2			; (calc'd as dwords per window*4) gives
	div	ecx			; the count of full windows, with the
	mov	[edi].wwincount,eax	; remainder being the number of bytes
	mov	[edi].wwinlbytes,edx	; in the last window (if any).

	;   set bank-switch vectors, then
	;   set both banks to -1, forces bankset on first screen access

	lea	eax,set_read_bank	; load address of set_read_bank,
	mov	[edi].wsrbvector,eax	; store it in pj_vdrv_wcontrol.
	lea	eax,set_write_bank	; load address of set_write_bank,
	mov	[edi].wswbvector,eax	; store it in pj_vdrv_wcontrol.

	mov	eax,-1			; set bank number 0...
	mov	[edi].wrdcurbank,eax	; current read bank = -1
	mov	[edi].wwrcurbank,eax	; current write bank = -1

	;   go make table of y offsets/splits/bank numbers

	call	_pj_vdrv_build_ytable

#punt:
	mov	eax,ebp
	ret

_pj_vesa_setmode   endp

;*****************************************************************************
;* Errcode pj_vesa_setmode(SMInfo *psmode) - called from C.
;*   Sets the vesa video mode described by *psmode.  Returns a negative
;*   error code on failure, or a longword value that will be passed to the
;*   corresponding clrmode() routine (if any) when the device is closed.
;*****************************************************************************

pj_vesa_setmode proc near

	Entry
	Args	#psmode
	Save	ebx,esi,edi,ebp

	mov	esi,#psmode		; load pointer to SMInfo for mode.
	lea	edi,pj_vdrv_wcontrol	; vectored setmode routines use this.
	movzx	ecx,[esi].smi_width	; most all cards will want the width,
	mov	[edi].wwidth,ecx	; pitch, and height values in the
	mov	[edi].wpitch,ecx	; wcontrol structure set the same as
	movzx	edx,[esi].smi_height	; the values in the SMInfo, so we do
	mov	[edi].wheight,edx	; that for them (they can override).
	call	_pj_vesa_setmode	; call the set-mode routine.
#done:
	Restore ebx,esi,edi,ebp
	Exit

pj_vesa_setmode endp

;*****************************************************************************
;* void pj_vesa_clrmode(long old_mode_data) - called from C.
;*****************************************************************************

pj_vesa_clrmode proc near

	Entry
	Args	#old_mode_data
	Save	ebx,esi,edi,ebp

	mov	esi,#old_mode_data
	lea	edi,pj_vdrv_wcontrol
	call	_pj_vesa_clrmode
#done:
	Restore ebx,esi,edi,ebp
	Exit

pj_vesa_clrmode endp

;*****************************************************************************
;* Errcode pj_vesa_detect(void) - called from C or asm.
;*
;* This routine will detect the presence of the VESA BIOS. If not present,
;* or if a DOS memory buffer can't be allocated, the appropriate error code
;* is returned.  If VESA is present, this routine will query the BIOS on the
;* video modes supported.  It will store the list of available modes into
;* the mode_array global array, then return the count of packed pixel modes.
;*
;* This routine is called both from C (in the vesa driver) and from asm (in
;* the svga autodetect driver).  When called from asm, the sminfo pointer is
;* passed in esi, and all regs have already been saved, but we save the
;* regs and load esi locally here anyway, in case we were called from C.
;* Since this is a one-shot routine, performance is not an issue.
;*****************************************************************************

pj_vesa_detect	proc  near

	Entry
	Save	ebx,esi,edi,ebp

	call	alloc_info_buffer	; get a buffer for VBE info calls.
	test	eax,eax 		; test return value, if not Success,
	jnz	#return 		; go return error code.

	mov	di,info_buffer.dosseg	; load realmode buffer segment number.
	mov	eax,VESA_VBE_INFO	; vesa 'get info' function,
	call	_pj_vesa_int10		; do it.

	cmp	ax,VSUCCESSFUL		; vesa call successfull?
	jne	short #not_vesa 	; if not, punt.

	; move the bios information into local 'bios_info' for easy access

	push	ds
	lds	esi,info_buffer.far32
	lea	edi,bios_info
	mov	ecx,(SIZE bios_info)/4
	rep movsd
	pop	ds

	; now double-check the local buffer for the VESA validation dword

	lea	eax,bios_info
	cmp	dptr [eax],'ASEV'       ; InfoBlock 1st word s/b be 'VESA' (our
	jne	short #not_vesa 	; consant is backwords for dword cmp).

	; it is vesa for sure, move the video mode array into 'mode_array'

	push	ds
	movzx	esi,word ptr bios_info.vi_ModesPtr+2
	shl	esi,4
	movzx	eax,word ptr bios_info.vi_ModesPtr
	add	esi,eax
	mov	ax,gs
	mov	ds,ax
	lea	edi,mode_array
	mov	ecx,MAXVMODES/2
	rep movsd
	pop	ds

	; call C function to filter mode data and build SMInfo array

	lea	eax,mode_array		; load pointer to vesamodes array.
	lea	esi,pj_vdrv_modeinfo	; load pointer to modeinfo array.
	push	eax			; push pointer to vesamodes array.
	push	esi			; push pointer to SMInfo array
	call	pj_vesa_build_sminfo	; do it (mode count returned in eax).
	add	esp,8
	jmp	short #return

#not_vesa:
	call	pj_vesa_free_dosbuf
	xor	eax,eax
#return:
	Restore ebx,esi,edi,ebp
	Exit

pj_vesa_detect	endp

;*****************************************************************************
; void pj_vesa_free_dosbuf(void) - called from C or asm.
;
;  Call the extender-specific function to free the dos realmode (below 1mb
;  line) buffer we use in communicating with VESA.  This is called from the
;  rex-layer cleanup routine via the vcleanup vector variable in svgaintf.asm,
;  to ensure the buffer is always freed before the driver is unloaded.
;*****************************************************************************

pj_vesa_free_dosbuf proc near

	lea	eax,info_buffer 	; load pointer to structure,
	push	eax			; push it, call dos memory free
	call	pj_dosmem_free		; routine (it handles NULL buffer
	add	esp,4			; pointer if we don't have buffer
	ret				; allocated.)

pj_vesa_free_dosbuf endp

_TEXT	ends
	end


;*****************************************************************************
;* DRVCOMN.I - Assembler-language header file for driver asm modules.
;*
;*  NOTES:
;*		This file is mirrored in many aspects by DRVCOMN.H for the C
;*		source code.  Changes in either header file will generally
;*		require corresponding changes in the other, followed by
;*		recompiling everything.
;*
;*		The driver asm code will look best if you set your
;*		editor's tab increment/size to 8.
;*
;*  MAINTENANCE:
;*    03/27/91	Ian Lepore
;*		Basically a total re-write.
;*    05/28/91	Ian Lepore
;*		Split original VESA.I file into DRVCOMN.I and VESA.I, to
;*		make it easier to clone the driver code.
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

PHAR_REAL_SEG	equ 34h

;-----------------------------------------------------------------------------
; NOTE:  If the value for LCLBUF_SIZE is changed, the corresponding constant
;	 in the C header file DRVCOMN.H must be changed to match!
;-----------------------------------------------------------------------------

LCLBUF_SIZE	equ 16384		; size of malloc'd local buffer
R_GRCLIB	equ R_HDW+0		; our first raster field after rasthdr

;*----------------------------------------------------------------------------
;* our main control structure...
;*   a global one of these is defined with the name 'pj_vdrv_wcontrol' in
;*   drvcomn.asm -- it holds everything we need to know while driving.
;*
;*   while this is ostensibly our 'window control' structure, we keep a
;*   few other handy values here, like the pointer to the local blit buffer,
;*   and the parms passed to the mask blits, and so on.  this is done mostly
;*   because every driver routine will have a pointer to this structure
;*   registerized, so it's pretty efficient to keep *everything* in it.
;*----------------------------------------------------------------------------

winblk		struc

woffsmask  dd	   0			; mask makes vid addr into window offset
wrdcurbank dd	   0			; current bank for read window
wwrcurbank dd	   0			; current bank for write window
wrdaddr    dd	   0			; base address of read window
wwraddr    dd	   0			; base address of write window
wpitch	   dd	   0			; line-to-line pitch
wrdwhich   dd	   0			; which window reads, A (0) or B (1)?
wwrwhich   dd	   0			; which window writes, A or B?
wwidth	   dd	   0			; screen width
wheight    dd	   0			; screen height
wwincount  dd	   0			; number of windows for full screen
wwindwords dd	   0			; number of dwords in a normal window
wwinlbytes dd	   0			; number of bytes in the last window
wsrbvector dd	   0			; pointer to set_read_bank function
wswbvector dd	   0			; pointer to set_write_bank function
wdrvdat1   dd	   0			; driver-specific data (for svga code)
wbankshift db	   0			; shift amount to make offset into bank
wgranshift db	   0			; shift amount to make bank into window
wsamerw    db	   0			; same window for read & write?
walign	   db	   0			; filler to align to dword boundry

wlocalbuf  dd	   0			; pointer to local (alloc'd) blit buffer

mbytes	   dd	   0			; maskXblit pointer to input bits
rx	   dd	   0			; maskXblit raster x
ry	   dd	   0			; maskXblit raster y
count	   dd	   0			; maskXblit output count/width
bit1	   db	   0			; maskXblit first bit to test in mbytes
oncolor    db	   0			; maskXblit color for on bits
offcolor   db	   0			; mask2blit color for off bits

winblk	   ends

;-----------------------------------------------------------------------------
; sminfo structure -- used to associate our modes with the hardware modes.
;-----------------------------------------------------------------------------

sminfo	struc
smi_mode   dw	?
smi_width  dw	?
smi_height dw	?
sminfo	ends

MAX_SMODES equ	16

;-----------------------------------------------------------------------------
; Set Read/Write Bank macros -- just shorthand for a vectored call statement.
;-----------------------------------------------------------------------------

SetReadBank macro
	call	dword ptr [edx].wsrbvector
	endm

SetWriteBank macro
	call	dword ptr [edx].wswbvector
	endm

;-----------------------------------------------------------------------------
; name our global data and internal-entry points.
;-----------------------------------------------------------------------------

ytab_bank equ	pj_vdrv_ytable		; these must appear before the extrn
ytab_offs equ	pj_vdrv_ytable+4	; below; Phar Lap's dumb fwd ref junk!

	extrn	pj_vdrv_modeinfo:sminfo
	extrn	pj_vdrv_wcontrol:winblk
	extrn	pj_vdrv_ytable:dword
	extrn	pj_vdrv_has_8bitdac:dword

	extrn	_pj_vdrv_build_ytable:near	; in drvcomn.asm
	extrn	_pj_vdrv_put_hseg:near		; in drvsegs.asm
	extrn	_pj_vdrv_get_hseg:near		; in drvsegs.asm

; end of drvcomn.i

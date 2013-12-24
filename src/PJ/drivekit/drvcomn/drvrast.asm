;*****************************************************************************
;* DRVRAST.ASM - set_rast, and xor_to/from_ram functions for the vesa driver.
;*
;*  NOTES:
;*		These functions are lumped together here because they all
;*		process an entire raster using bank-at-a-time logic instead
;*		of the usual line-at-a-time.
;*
;*  MAINTENANCE:
;*    03/27/91	Ian Lepore
;*		Basically a total re-write.
;*    06/19/91	Ian
;*		Fixed a bug in xor_from/to_ram() routines...if the display
;*		pitch value did not equal the bytemap's bpr value, the routine
;*		would glitch out.  Now it will punt off to the generic
;*		routines if the pitch & bpr don't match.  (In reality, I don't
;*		expect we'll get many vesa/svga cards where pitch != width,
;*		unless the trick I found of reprogramming the VGA OFFSET
;*		register becomes common.)
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
	include raster.i
	include rastlib.i

_TEXT	segment

	public	pj_vdrv_set_rast
	public	pj_vdrv_xor_to_ram
	public	pj_vdrv_xor_from_ram

;*****************************************************************************
;* void pj_vdrv_set_rast(Raster *rast, Pixel color);
;*****************************************************************************

pj_vdrv_set_rast proc near

	Entry
	Lclvars #color4
	Args	#rast,#color
	Save	ebx,edi,es

	mov	ax,gs
	mov	es,ax

	mov	dl,#color		; put color into all 4 bytes of eax...
	mov	dh,dl
	shrd	eax,edx,16
	mov	ax,dx
	mov	#color4,eax		; save color replicated into dword.

	lea	edx,pj_vdrv_wcontrol
	xor	ebx,ebx
#loop:
	mov	eax,ebx 		; flood each bank with color...
	SetWriteBank
	mov	ecx,[edx].wwindwords
	mov	edi,[edx].wwraddr
	mov	eax,#color4
	rep stosd
	inc	ebx
	cmp	ebx,[edx].wwincount
	jb	#loop

	mov	ecx,[edx].wwinlbytes	; flood last bank...
	test	ecx,ecx
	jz	short #done
	mov	eax,ebx
	SetWriteBank
	mov	edi,[edx].wwraddr
	mov	eax,#color4
	test	ecx,3
	jnz	short #slow
	shr	ecx,2
	rep stosd
	jmp	short #done
#slow:
	rep stosb
#done:
	Restore ebx,edi,es
	Exit

pj_vdrv_set_rast endp

;*****************************************************************************
;* void pj_vdrv_xor_to_ram(Raster *srast, Bytemap *drast);
;*****************************************************************************

pj_vdrv_xor_to_ram proc near

	Entry
	Args	#srast,#drast

	mov	eax,#drast		; our algorithm counts on the hardware
	mov	ecx,[eax].bym_bpr	; raster bpr (pitch) equaling the
	cmp	ecx,pj_vdrv_wcontrol.wpitch	; bytemap bpr.	compare the two values,
	je	short #can_do		; and if equal, continue at our code
	mov	eax,#srast		; below, else punt off to the generic
	mov	eax,[eax].R_GRCLIB	; routines.
	jmp	dptr [eax].RL_XOR_RAST+RL_TO_BMAP

	align 4
#can_do:

	Save	ebx,esi,edi

	mov	eax,#drast
	mov	edi,[eax].bym_p
	lea	edx,pj_vdrv_wcontrol
	xor	ebx,ebx
#mainloop:
	mov	eax,ebx
	SetReadBank
	mov	ecx,[edx].wwindwords
	mov	esi,[edx].wrdaddr
#winloop:
	lods	dptr gs:[esi]		; xor each full bank,
	xor	[edi],eax		; working a dword at a time...
	add	edi,4
	dec	ecx
	jnz	short #winloop

	inc	ebx
	cmp	ebx,[edx].wwincount
	jb	#mainloop

	mov	ecx,[edx].wwinlbytes	; xor the last bank...
	test	ecx,ecx
	jz	short #done
	mov	eax,ebx
	SetReadBank
	mov	esi,[edx].wrdaddr
	test	ecx,3
	jnz	short #slow
	shr	ecx,2
#winloop1:
	lods	dptr gs:[esi]
	xor	[edi],eax
	add	edi,4
	dec	ecx
	jnz	short #winloop1
	jmp	short #done
#slow:
	lods	bptr gs:[esi]
	xor	[edi],al
	inc	edi
	dec	ecx
	jnz	short #slow
#done:
	Restore ebx,esi,edi
	Exit

pj_vdrv_xor_to_ram endp


;*****************************************************************************
;* void pj_vdrv_xor_from_ram(Bytemap *srast, Raster *drast);
;*****************************************************************************

pj_vdrv_xor_from_ram proc near

	Entry
	Args	#srast,#drast

	mov	eax,#srast		; our algorithm counts on the hardware
	mov	ecx,[eax].bym_bpr	; raster bpr (pitch) equaling the
	cmp	ecx,pj_vdrv_wcontrol.wpitch	; bytemap bpr.	compare the two values,
	je	short #can_do		; and if equal, continue at our code
	mov	eax,#drast		; below, else punt off to the generic
	mov	eax,[eax].R_GRCLIB	; routines.
	jmp	dptr [eax].RL_XOR_RAST+RL_FROM_BMAP

	align 4
#can_do:

	Save	ebx,edi,esi

	mov	eax,#srast
	mov	esi,[eax].bym_p
	lea	edx,pj_vdrv_wcontrol
	xor	ebx,ebx
#mainloop:
	mov	eax,ebx
	SetReadBank
	mov	ecx,[edx].wwindwords
	mov	edi,[edx].wrdaddr
#winloop:
	lodsd
	xor	gs:[edi],eax
	add	edi,4
	dec	ecx
	jnz	short #winloop

	inc	ebx
	cmp	ebx,[edx].wwincount
	jb	#mainloop

	mov	ecx,[edx].wwinlbytes
	test	ecx,ecx
	jz	short #done
	mov	eax,ebx
	SetReadBank
	mov	edi,[edx].wrdaddr
	test	ecx,3
	jnz	short #slow
	shr	ecx,2
#winloop1:
	lodsd
	xor	gs:[edi],eax
	add	edi,4
	dec	ecx
	jnz	short #winloop1
	jmp	short #done
#slow:
	lodsb
	xor	gs:[edi],al
	inc	edi
	dec	ecx
	jnz	short #slow
#done:
	Restore ebx,edi,esi
	Exit

pj_vdrv_xor_from_ram endp


_TEXT	ends
	end


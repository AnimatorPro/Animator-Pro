;*****************************************************************************
;* DRVSEGS.ASM - get/put_hseg, and get/put_rectpix driver functions.
;*
;*  NOTES:
;*		This module contains entry points used by other DRVxxxx.ASM
;*		modules, as well as routines provided to the host.
;*
;*		For what it's worth, the routines in this module are the
;*		most performance-critical functions within the driver.
;*
;*  MAINTENANCE:
;*    03/27/91	Ian Lepore
;*		Basically a total re-write.
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

_TEXT	segment

	public	_pj_vdrv_put_hseg	       ; entry point for other ASM routines
	public	_pj_vdrv_get_hseg	       ; entry point fo rother ASM routines

	public	pj_vdrv_put_hseg
	public	pj_vdrv_get_hseg
	public	pj_vdrv_put_rectpix
	public	pj_vdrv_get_rectpix


;*****************************************************************************
;* _pj_vdrv_put_hseg - internal service routine
;* Entry:
;*	 eax =	y coordinate
;*	 ecx =	width
;*	 edx -> pj_vdrv_wcontrol
;*	 edi =	x coordinate
;*	 esi -> input buffer
;*	 es  =	0x0034 (DOS memory segment)
;* Exit:
;*	 eax - trashed
;*	 ecx - trashed
;*	 esi -> next byte in input buffer
;*	 edi - trashed
;*	 all others preserved
;*****************************************************************************

	align 4
_pj_vdrv_put_hseg proc near

	add	edi,[eax*8+ytab_offs]	; 4
	mov	eax,[eax*8+ytab_bank]	; 4
	cmp	eax,[edx].wwrcurbank	; 6
	jne	short #newbank		; 3
#putseg:				;
	add	edi,[edx].wwraddr	; 6
	mov	al,cl			; 2 do typical fastmove stuff...
	and	al,3			; 2
	shr	ecx,2			; 3
	rep movsd			; X
	mov	cl,al			; 2
	rep movsb			; X
	ret

	align 4
#newbank:
	test	eax,0FFFF0000h		; is this a split line?
	jnz	short #splitline	; yep, go handle it.
	SetWriteBank			; nope, just set the new write bank
	jmp	short #putseg		; then continue.

	align 4
#splitline:
	and	eax,0000FFFFh		; clean split_at value from eax.
	cmp	edi,[edx].woffsmask	; is the starting X address after the
	jbe	short #checkend 	; split?  if not, go check the ending
	and	edi,[edx].woffsmask	; address, else increment to the next
	inc	eax			; (post-split) bank number, mask the
	cmp	eax,[edx].wwrcurbank	; address to the new bank.  if the
	je	short #putseg		; new bank is the current bank, we're
	SetWriteBank			; all set, else set the new write
	jmp	short #putseg		; bank, then continue.

	align 4
#checkend:
	cmp	eax,[edx].wwrcurbank	; we have to do at least some output
	je	short #ok_bank		; before the split, make sure we're
	SetWriteBank			; in the right bank before proceesing.
#ok_bank:
	lea	eax,[ecx+edi-1] 	; make a pointer to the last input byte,
	sub	eax,[edx].woffsmask	; compare it to the split mask. if all
	jbe	short #putseg		; input before split, go do normal move.

	sub	ecx,eax 		; counts: eax=postsplit, ecx=presplit.
	add	edi,[edx].wwraddr	; add video address to input pointer.
	shr	ecx,1			; move the pre-split data...
	rep movsw			;
	adc	ecx,0			;
	rep movsb			;
	mov	ecx,eax 		; restore the post-split count to its
	xor	edi,edi 		; normal register.  adjust the input
	mov	eax,[edx].wwrcurbank	; pointer to the new bank.  put the
	inc	eax			; current bank number, increment to
	SetWriteBank			; the next bank, go switch banks.
	jmp	short #putseg		; handle post-split data as normal.

_pj_vdrv_put_hseg endp

;*****************************************************************************
;* _pj_vdrv_get_hseg - internal service routine
;* Entry:
;*	 eax =	y coordinate
;*	 ecx =	width
;*	 edx -> pj_vdrv_wcontrol
;*	 esi =	x coordinate
;*	 edi -> output buffer
;*	 es  =	0x0014 (normal data segment for pharlap)
;* Exit:
;*	 eax - trashed
;*	 ecx - trashed
;*	 esi - trashed
;*	 edi -> next byte in output buffer
;*	 all others preserved
;*****************************************************************************

	align 4
_pj_vdrv_get_hseg proc near

	add	esi,[eax*8+ytab_offs]	; 4
	mov	eax,[eax*8+ytab_bank]	; 4
	cmp	eax,[edx].wrdcurbank	; 6
	jne	short #newbank		; 3
#getseg:				;
	add	esi,[edx].wrdaddr	; 6
	mov	al,cl			; 2 do typical fastmove stuff...
	and	al,3			; 2
	shr	ecx,2			; 3
	rep movs dptr es:[edi],gs:[esi] ; X
	mov	cl,al			; 2
	rep movs bptr es:[edi],gs:[esi] ; X
	ret

	align 4
#newbank:
	test	eax,0FFFF0000h		; is this a split line?
	jnz	short #splitline	; yep, go handle it.
	SetReadBank			; nope, just set the new read bank
	jmp	short #getseg		; then continue.

	align 4
#splitline:
	and	eax,0000FFFFh		; clean split_at value from eax.
	cmp	esi,[edx].woffsmask	; is the starting X address after the
	jbe	short #checkend 	; split?  if not, go check the ending
	and	esi,[edx].woffsmask	; address, else increment to the next
	inc	eax			; (post-split) bank number, mask the
	cmp	eax,[edx].wrdcurbank	; address to the new bank.  if the
	je	short #getseg		; new bank is the current bank, we're
	SetReadBank			; all set, else set the new read
	jmp	short #getseg		; bank, then continue.

	align 4
#checkend:
	cmp	eax,[edx].wrdcurbank	; we have to do at least some output
	je	short #ok_bank		; before the split, make sure we're
	SetReadBank			; in the right bank before proceeding.
#ok_bank:
	lea	eax,[ecx+esi-1] 	; make a pointer to the last input byte,
	sub	eax,[edx].woffsmask	; compare it to the split mask. if all
	jbe	short #getseg		; input before split, go do normal move.

	sub	ecx,eax 		; counts: eax=postsplit, ecx=presplit.
	add	esi,[edx].wrdaddr	; add video address to input pointer.
	shr	ecx,1			; move the pre-split data...
	rep movs wptr es:[edi],gs:[esi] ;
	adc	ecx,0			;
	rep movs bptr es:[edi],gs:[esi] ;
	mov	ecx,eax 		; restore the post-split count to its
	xor	esi,esi 		; normal register.  adjust the input
	mov	eax,[edx].wrdcurbank	; pointer to the new bank.  get the
	inc	eax			; current bank number, increment to
	SetReadBank			; the next bank, go switch banks.
	jmp	short #getseg		; handle post-split data as normal.

_pj_vdrv_get_hseg endp


;*****************************************************************************
;* void pj_vdrv_put_hseg(Raster *r, Pixel pixbuf, int x, int y, int w)
;*****************************************************************************

	align 4
pj_vdrv_put_hseg proc near

	Entry
	Args	#raster,#pixbuf,#x,#y,#w
	Save	esi,edi,es

	mov	ax,gs
	mov	es,ax

	lea	edx,pj_vdrv_wcontrol		; 2
	mov	ecx,#w			; 4
	mov	eax,#y			; 4
	mov	edi,#x			; 4
	mov	esi,#pixbuf		; 4
	call	_pj_vdrv_put_hseg

	Restore esi,edi,es
	Exit

pj_vdrv_put_hseg endp

;*****************************************************************************
;* void pj_vdrv_get_hseg(Raster *r, Pixel pixbuf, int x, int y, int w)
;*****************************************************************************

	align 4
pj_vdrv_get_hseg proc near

	Entry
	Args	#raster,#pixbuf,#x,#y,#w
	Save	esi,edi

	lea	edx,pj_vdrv_wcontrol		; 2
	mov	ecx,#w			; 4
	mov	eax,#y			; 4
	mov	esi,#x			; 4
	mov	edi,#pixbuf		; 4
	call	_pj_vdrv_get_hseg

	Restore esi,edi
	Exit

pj_vdrv_get_hseg endp

;*****************************************************************************
;* void pj_vdrv_put_rectpix(Raster *rast, Pixel *pixbuf, int x,y,w,h);
;*****************************************************************************

	align 4
pj_vdrv_put_rectpix proc near

	Entry
	Args	#rast,#pixbuf,#x,#y,#w,#h
	Save	ebx,esi,edi,ebp,es

	mov	ax,gs
	mov	es,ax

	mov	ebp,#h
	mov	ebx,#y
	mov	esi,#pixbuf
	lea	edx,pj_vdrv_wcontrol
#loop:					; load parm regs...
	mov	ecx,#w			; width
	mov	eax,ebx 		; y
	mov	edi,#x			; x
	call	_pj_vdrv_put_hseg
	inc	ebx
	dec	ebp
	jnz	#loop

	Restore ebx,esi,edi,ebp,es
	Exit

pj_vdrv_put_rectpix endp


;*****************************************************************************
;*
;*****************************************************************************

	align 4
pj_vdrv_get_rectpix proc near

	Entry
	Args	#rast,#pixbuf,#x,#y,#w,#h
	Save	ebx,esi,edi,ebp

	mov	ebp,#h
	mov	ebx,#y
	mov	edi,#pixbuf
	lea	edx,pj_vdrv_wcontrol
#loop:					; load parm regs...
	mov	ecx,#w			; width
	mov	eax,ebx 		; y
	mov	esi,#x			; x
	call	_pj_vdrv_get_hseg
	inc	ebx
	dec	ebp
	jnz	#loop

	Restore ebx,esi,edi,ebp
	Exit

pj_vdrv_get_rectpix endp

_TEXT	ends
	end


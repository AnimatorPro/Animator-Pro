;*****************************************************************************
;* DRVDOTS.ASM - Get/Put Dot driver routines.
;*
;*  NOTES:
;*
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

	public	pj_vdrv_put_dot
	public	pj_vdrv_cput_dot
	public	pj_vdrv_get_dot
	public	pj_vdrv_cget_dot

;*****************************************************************************
;* void pj_vdrv_cput_dot(Raster *r, int color, int x, int y)  - clipped dot
;* void pj_vdrv_put_dot(Raster *r, int color, int x, int y)   - unclipped dot
;*****************************************************************************

	align 4
pj_vdrv_cput_dot proc near		       ; entry point for clipped output

	Entry
	Args	#rast,#color,#x,#y

	lea	edx,pj_vdrv_wcontrol		; load pointer to global window control.
	mov	ecx,#x			; get requested X coordinate, compare
	cmp	ecx,[edx].wwidth	; it to width (unsigned compare, neg
	jae	short #punt		; X values appear too large and get
	mov	ecx,#y			; clipped as they should.)  same deal
	cmp	ecx,[edx].wheight	; with Y coordinate.  if both are in
	jb	short vpd_go		; range, go do the screen output.
#punt:
	Exit

	align 4
pj_vdrv_put_dot  proc near		       ; entry point for unclipped output

	Entry
	Args	#rast,#color,#x,#y

	mov	ecx,#y		     ;4 ; load y coordinate
	lea	edx,pj_vdrv_wcontrol	     ;2 ; load window control pointer
vpd_go:
	mov	eax,[ecx*8+ytab_bank]   ;4 ; get split_at/banknum from ytable,
	mov	ecx,[ecx*8+ytab_offs] ;4 ; get offset-within-bank from ytable.
	add	ecx,#x		     ;6 ; add X to offset.
	cmp	eax,[edx].wwrcurbank ;6 ; compare current bank to needed bank,
	jne	short #checksplit    ;3 ; if not equal, go check for split line.
#putdot:
	add	ecx,[edx].wwraddr    ;6 ; add base of screen memory, giving
	mov	al,#color	     ;4 ; output pointer. load pixel and
	mov	gs:[ecx],al	     ;2 ; write it to the screen.
	Exit			     ;=41 cycles if no bankswitch.

	align 4
#checksplit:
	movzx	eax,ax		     ;3 ; wipe out split-at value; don't need
	push	ecx		     ;2 ; it for dot output.  save ecx for a
	mov	cl,[edx].wbankshift  ;4 ; mo.  if the line was split, we need
	shr	ecx,cl		     ;3 ; to incr bank num if output is after
	add	eax,ecx 	     ;2 ; split, this code does that. restore
	pop	ecx		     ;4 ; ecx after calcing proper output bank.
	and	ecx,[edx].woffsmask  ;6 ; in case output is after the split,
	cmp	eax,[edx].wwrcurbank ;6 ; mask off the bank number in the
	je	short #putdot	     ;	; output pointer.  if the actual bank
	SetWriteBank		     ;	; number for output is not the current
	jmp	short #putdot	     ;	; bank, go set the new write bank.

pj_vdrv_put_dot  endp
pj_vdrv_cput_dot endp

;*****************************************************************************
;* void pj_vdrv_cget_dot(Raster *r, int x, int y)
;* void pj_vdrv_get_dot(Raster *r, int x, int y)
;*****************************************************************************

	align 4
pj_vdrv_cget_dot proc near

	Entry
	Args	#rast,#x,#y

	lea	edx,pj_vdrv_wcontrol		; load pointer to global window control.
	mov	ecx,#x			; get requested X coordinate, compare
	cmp	ecx,[edx].wwidth	; it to width (unsigned compare, neg
	jae	short #punt		; X values appear too large and get
	mov	ecx,#y			; clipped as they should.)  same deal
	cmp	ecx,[edx].wheight	; with Y coordinate.  if both are in
	jb	short vgd_go		; range, go do the screen output.
#punt:
	xor	eax,eax 		; point was clipped, must return 0.
	Exit

	align 4
pj_vdrv_get_dot  proc near

	Entry
	Args	#rast,#x,#y

	mov	ecx,#y			; 4 (see comments in put_dot, above)
	lea	edx,pj_vdrv_wcontrol		; 2
vgd_go:
	mov	eax,[ecx*8+ytab_bank]	; 4
	mov	ecx,[ecx*8+ytab_offs]	; 4
	add	ecx,#x			; 6
	cmp	eax,[edx].wrdcurbank	; 6
	jne	short #checksplit	; 3
#getdot:
	add	ecx,[edx].wrdaddr	; 6
	movzx	eax,byte ptr gs:[ecx]	; 5
	Exit				; =40 cycles if no bankswitch.

#checksplit:
	movzx	eax,ax			; 3
	push	ecx			; 2
	mov	cl,[edx].wbankshift	; 4
	shr	ecx,cl			; 3
	add	eax,ecx 		; 2
	pop	ecx			; 4
	and	ecx,[edx].woffsmask	; 6
	cmp	eax,[edx].wrdcurbank	; 6
	je	short #getdot		; 3 or 10
	SetReadBank			; lots
	jmp	short #getdot		; 10

pj_vdrv_get_dot  endp
pj_vdrv_cget_dot endp

_TEXT	ends
	end


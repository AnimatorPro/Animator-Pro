;*****************************************************************************
;* DRVXORR.ASM - The xor_rect driver function.
;*
;*  NOTES:
;*		The xor_rect routine below is optimized for small rectangles;
;*		if the rect is bigger than our alloc'd blit buffer, we punt
;*		to the generic library routine.  In practice, this should
;*		virtually never happen:  about the only use PJ makes of the
;*		xor_rect routine these days is for blinking the cursor in
;*		the file selector and other input boxes; the cursor will
;*		always fit within our (16k) blit buffer.  The TORTURE program
;*		reports mediocre performance for this routine, because it
;*		XORs 5 very large rectangles before getting down the the more
;*		realistically-sized stuff.  (I didn't know any better when I
;*		wrote TORTURE.)
;*
;*  MAINTENANCE:
;*    03/27/91	Ian Lepore
;*		Basically a total re-write.
;*    06/05/91	Ian
;*		Unrolled the inner loop a bit.	(What the heck...why not?)
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

	extrn	pj_vdrv_get_rectpix:near
	extrn	pj_vdrv_put_rectpix:near
	public	pj_vdrv_xor_rect

;*****************************************************************************
; void pj_vdrv_xor_rect(Raster *r, Pixel color, int x, int y, int w, int h)
;
; Note:
;	There's a tiny bit of cheapness here.  When the entire rect to be XORed
;	fits in our local buffer, we get the whole rect, XOR it all at once,
;	then put it back.  When XORing the buffer, we do it a dword at a time,
;	and we don't care if the buffer isn't holding an even dword multiple
;	of bytes.  If it isn't, we just end up XORing a few crufty bytes
;	at the end of the buffer which don't get written back to the screen
;	anyway.
;*****************************************************************************

	align 4
pj_vdrv_xor_rect proc near

	mov	eax,[esp+20]
	mul	dptr [esp+24]
	cmp	eax,LCLBUF_SIZE
	jbe	#can_do
	mov	eax,[esp+4]
	mov	eax,[eax].R_GRCLIB
	jmp	dptr [eax].RL_XOR_RECT

#can_do:

	Entry
	BArgs	#raster,#color,#x,#y,#w,#h
	Save	edi

	push	eax			; save length of buffer for xor loop.
	mov	edi,pj_vdrv_wcontrol.wlocalbuf	; load pointer to local buffer.

	push	#h			; call our get_rectpix routine.
	push	#w
	push	#y
	push	#x
	push	edi
	push	#raster
	call	pj_vdrv_get_rectpix
	add	esp,24

	pop	ecx			; get count of bytes in buffer, round
	add	ecx,3			; up to nearest dword, then adjust
	shr	ecx,2			; byte count to dword count.

	mov	al,#color		; load xor color into all four bytes
	mov	ah,al			; of edx...
	shrd	edx,eax,16
	mov	dx,ax
	mov	eax,edi
#xorloop:
	xor	[eax],edx
	add	eax,4
	dec	ecx
	jz	short #xordone
	xor	[eax],edx
	add	eax,4
	dec	ecx
	jz	short #xordone
	xor	[eax],edx
	add	eax,4
	dec	ecx
	jz	short #xordone
	xor	[eax],edx
	add	eax,4
	dec	ecx
	jnz	short #xorloop

#xordone:
	push	#h			; call our put_rectpix routine.
	push	#w
	push	#y
	push	#x
	push	edi
	push	#raster
	call	pj_vdrv_put_rectpix
	add	esp,24

	Restore edi
	Exit

pj_vdrv_xor_rect endp

_TEXT	ends
	end


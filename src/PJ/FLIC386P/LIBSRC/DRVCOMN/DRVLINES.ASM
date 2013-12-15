;*****************************************************************************
;* DRVLINE.ASM - set_hline and set_rect functions for the vesa driver.
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

	public	pj_vdrv_set_hline
	public	pj_vdrv_set_rect

;*****************************************************************************
;* void pj_vdrv_set_hline(Raster *r, Pixel color, int x, int y, int w)
;*****************************************************************************

	align 4
pj_vdrv_set_hline proc near

	Entry
	Args	#raster,#color,#x,#y,#w
	Save	edi,es

	mov	ax,gs
	mov	es,ax

	lea	edx,pj_vdrv_wcontrol		; 2
	mov	edi,#y			; 4
	mov	eax,[edi*8+ytab_bank]	; 4
	mov	edi,[edi*8+ytab_offs]	; 4
	add	edi,#x			; 6
#nosplit:
	cmp	eax,[edx].wwrcurbank	; 6
	jne	short #newbank		; 3
#setline:				;
	mov	ecx,#w			; 4
	add	edi,[edx].wwraddr	; 6 = 39

	mov	dl,#color		; 4 replicate color byte into all bytes
	mov	dh,dl			; 2 of eax...
	shrd	eax,edx,16		; 3
	mov	ax,dx			; 2

	mov	dl,cl			; 2 do typical fastmove stuff...
	and	dl,3			; 2
	shr	ecx,2			; 3
	rep stosd			; X
	mov	cl,dl			; 2
	rep stosb			; X

	Restore edi,es
	Exit

	align 4
#newbank:
	test	eax,0FFFF0000h
	jnz	short #splitline
	SetWriteBank
	jmp	short #setline

	align 4
#splitline:
	ror	eax,16			; swap split_at and bank number in eax.
	mov	ecx,#x			; load starting x.
	cmp	cx,ax			; starting x after split?
	jb	short #checkend 	; nope, ahead of it, go check ending x.
	shr	eax,16			; yep, so the entire output is after
	inc	eax			; the split; increment to the next
	and	edi,[edx].woffsmask	; bank and adjust of offset within the
	jmp	short #nosplit		; bank, then continue as a normal line.
#checkend:
	sub	ax,cx			; get distance between start and split,
	cmp	ax,#w			; compare to length of output. if
	jb	short #truesplit	; output fits before split point, handle
	shr	eax,16			; as normal. (we've cleared split_at
	jmp	short #nosplit		; from upper bits of eax.)
#truesplit:
	mov	cx,ax			; true split, put pre-split move count
	sub	#w,ecx			; in ecx, adjust post-split count.
	shr	eax,16			; shift bank number back to ax.
	cmp	eax,[edx].wwrcurbank	;are we in the right bank now?
	je	short #split1		; yep, skip setting bank.
	SetWriteBank
#split1:
	add	edi,[edx].wwraddr
	mov	al,#color
	rep stosb
	mov	eax,[edx].wwrcurbank
	inc	eax
	SetWriteBank
	xor	edi,edi
	jmp	#setline

pj_vdrv_set_hline endp

;*****************************************************************************
;* void pj_vdrv_set_rect(Raster *r, Pixel color, int x, int y, int w, int h)
;*****************************************************************************

	align 4
pj_vdrv_set_rect proc near

	Entry
	Lclvars #widthd,#widthb
	Args	#raster,#color,#x,#y,#w,#h
	Save	ebx,ebp,esi,edi,es

	mov	ax,gs
	mov	es,ax

	mov	dl,#color		; 4 replicate color byte into all bytes
	mov	dh,dl			; 2 of esi...
	shrd	esi,edx,16		; 3
	mov	si,dx			; 2

	mov	ecx,#w
	mov	eax,ecx
	shr	eax,2
	mov	#widthd,eax
	and	ecx,3
	mov	#widthb,ecx

	lea	edx,pj_vdrv_wcontrol
	mov	eax,#y
	lea	ebp,[eax*8+ytab_bank]

	mov	ebx,[ebp+4]
	add	ebx,#x
	add	ebx,[edx].wwraddr
#lineloop:
	mov	eax,[ebp]
	cmp	eax,[edx].wwrcurbank
	jne	short #checkbank
#setline:
	mov	edi,ebx
	mov	eax,esi
	mov	ecx,#widthd
	rep stosd
	mov	ecx,#widthb
	rep stosb
	add	ebx,[edx].wpitch
#endloop:
	add	ebp,8
	dec	dptr #h
	jnz	short #lineloop
	Restore ebx,ebp,esi,edi,es
	Exit

#checkbank:
	test	eax,0FFFF0000h		; is this a split line? if not, we just
	jz	#setbank		; need to set the bank. if so, move the
	ror	eax,16			; split_at value to ax, compare it to
	cmp	ax,wptr #x		; the starting X coord. if start_x
	ja	#dosplit		; greater than split_at, the whole line
	shr	eax,16			; is after the split.  just move to the
	inc	eax			; next bank, then continue as normal.
#setbank:
	SetWriteBank			; set the new write bank.
	mov	ebx,[ebp+4]		; rebuild the current output pointer
	add	ebx,#x			; to correspond to the new bank.
	and	ebx,[edx].woffsmask
	add	ebx,[edx].wwraddr
	jmp	short #setline

#dosplit:

	shr	eax,16
	cmp	eax,[edx].wwrcurbank
	je	short #nosplitset
	SetWriteBank			; set the new write bank.
#nosplitset:
	mov	ebx,[edx].wwraddr	; load video base pointer.
	mov	edi,[ebp+4]		; build a current output pointer
	add	edi,#x			; without the video base address.
	mov	ecx,#w			; load output width.
	mov	eax,esi 		; load output pixel.
#splitloop:
	mov	es:[ebx+edi],al 	; store pixel.
	inc	edi			; increment output pointer.
	and	edi,[edx].woffsmask	; mask pointer to end of bank.
	jnz	short #notsplityet	; if zero, we've crossed the split
	mov	eax,[edx].wwrcurbank	; point, switch to the new bank.
	inc	eax
	SetWriteBank
	mov	eax,esi 		; reload output pixel.
#notsplityet:
	loop	#splitloop

	add	ebx,[ebp+12]		; offset for *next* line in ytable
	add	ebx,#x
	jmp	#endloop

pj_vdrv_set_rect endp

_TEXT	ends
	end


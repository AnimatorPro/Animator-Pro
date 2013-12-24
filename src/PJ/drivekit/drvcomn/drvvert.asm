;*****************************************************************************
;* DRVVERT.ASM - Get/Put/Set vertical segment/line driver functions.
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

	public	pj_vdrv_set_vline
	public	pj_vdrv_put_vseg
	public	pj_vdrv_get_vseg

;*****************************************************************************
;* void pj_vdrv_set_vline(Raster *rast, Pixel color, int x, int y, int h);
;*****************************************************************************

pj_vdrv_set_vline proc near

	Entry
	Args	#rast,#color,#x,#y,#h
	Save	ebx,esi,edi,ebp

	lea	edx,pj_vdrv_wcontrol
	mov	ebp,[edx].wpitch
	mov	esi,[edx].woffsmask
	mov	ebx,[edx].wwraddr
	mov	ecx,#h

	mov	eax,#y
	mov	edi,[eax*8+ytab_offs]
	add	edi,#x
	movzx	eax,wptr [eax*8+ytab_bank]
	cmp	edi,esi
	ja	short #nextbank
	cmp	eax,[edx].wwrcurbank
	jne	short #setbank
	mov	al,#color
#setline:
	dec	ecx
	js	short #done
	mov	gs:[ebx+edi],al
	add	edi,ebp
	cmp	edi,esi
	jbe	short #setline
	mov	eax,[edx].wwrcurbank
#nextbank:
	inc	eax
	and	edi,esi
#setbank:
	SetWriteBank
	mov	al,#color
	jmp	short #setline
#done:
	Restore ebx,esi,edi,ebp
	Exit

pj_vdrv_set_vline endp

;*****************************************************************************
;* void pj_vdrv_put_vseg(Raster *rast, Pixel *pixbuf, int x, int y, int h);
;*****************************************************************************

pj_vdrv_put_vseg proc near

	Entry
	Args	#rast,#pixbuf,#x,#y,#h
	Save	ebx,esi,edi,ebp

	lea	edx,pj_vdrv_wcontrol
	mov	ebp,[edx].wpitch
	mov	esi,#pixbuf
	mov	ebx,[edx].wwraddr
	mov	ecx,#h

	mov	eax,#y
	mov	edi,[eax*8+ytab_offs]
	add	edi,#x
	movzx	eax,wptr [eax*8+ytab_bank]
	cmp	edi,[edx].woffsmask
	ja	short #nextbank
	cmp	eax,[edx].wwrcurbank
	jne	short #setbank
#putseg:
	dec	ecx
	js	short #done
	lodsb
	mov	gs:[ebx+edi],al
	add	edi,ebp
	cmp	edi,[edx].woffsmask
	jbe	short #putseg
	mov	eax,[edx].wwrcurbank
#nextbank:
	inc	eax
	and	edi,[edx].woffsmask
#setbank:
	SetWriteBank
	jmp	short #putseg
#done:
	Restore ebx,esi,edi,ebp
	Exit

pj_vdrv_put_vseg endp

;*****************************************************************************
;* void pj_vdrv_get_vseg(Raster *rast, Pixel *pixbuf, int x, int y, int h);
;*****************************************************************************

pj_vdrv_get_vseg proc near

	Entry
	Args	#rast,#pixbuf,#x,#y,#h
	Save	ebx,edi,esi,ebp

	lea	edx,pj_vdrv_wcontrol
	mov	ebp,[edx].wpitch
	mov	edi,#pixbuf
	mov	ebx,[edx].wrdaddr
	mov	ecx,#h

	mov	eax,#y
	mov	esi,[eax*8+ytab_offs]
	add	esi,#x
	movzx	eax,wptr [eax*8+ytab_bank]
	cmp	esi,[edx].woffsmask
	ja	short #nextbank
	cmp	eax,[edx].wrdcurbank
	jne	short #setbank
#getseg:
	dec	ecx
	js	short #done
	mov	al,gs:[ebx+esi]
	stosb
	add	esi,ebp
	cmp	esi,[edx].woffsmask
	jbe	short #getseg
	mov	eax,[edx].wrdcurbank
#nextbank:
	inc	eax
	and	esi,[edx].woffsmask
#setbank:
	SetReadBank
	jmp	short #getseg
#done:
	Restore ebx,edi,esi,ebp
	Exit

pj_vdrv_get_vseg endp


_TEXT	ends
	end


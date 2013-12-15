;*****************************************************************************
;* DRVUNLC.ASM - The unlccomp_rect routine for the vesa driver.
;*
;*  NOTES:
;*		It may strike you that some of the instructions in this
;*		routine are arbitrarily placed.  They are <grin>.  This is
;*		to keep as many of the branches as possible in range for a
;*		short branch, to make the most common conditions take the
;*		'fall thru' cases, and to ensure that the 386 prefetch queue
;*		stays reasonably full by intermixing short and long-operand
;*		instructions where the given instructions can run in an
;*		arbitrary order.
;*
;*  MAINTENANCE:
;*    04/25/91	Ian Lepore
;*		Added this routine to the driver.
;*****************************************************************************

	include stdmacro.i
	include drvcomn.i

_TEXT	segment

	extrn	pj_vdrv_put_hseg:near
	extrn	pj_vdrv_set_hline:near

	public	pj_vdrv_unlccomp_rect

;*****************************************************************************
; void pj_vdrv_unlccomp_rect(Raster *r, void *ucbuf, LONG pixsize,
;			  Coor x, Coor y, Ucoor width, Ucoor
;*****************************************************************************

	align 4
pj_vdrv_unlccomp_rect proc near

	Entry
	Lclvars #linecount
	Args	#rast, #ucbuf, #pixsize, #x, #y, #w, #h
	Save	ebx,esi,edi,ebp,es

	mov	ax,gs
	mov	es,ax

	mov	esi,#ucbuf
	xor	eax,eax
	lea	edx,pj_vdrv_wcontrol
	xor	ecx,ecx
	mov	ebp,#y
	lodsw
	add	ebp,eax
	dec	ebp
	lodsw
	mov	#linecount,eax
	jmp	short #lineloop
#done:
	Restore ebx,esi,edi,ebp,es
	Exit

	align 4
#lineloop:
	inc	ebp
	dec	dptr #linecount
	js	short #done

	movzx	ebx,bptr [esi]
	inc	esi
	test	ebx,ebx
	jz	short #lineloop

	mov	edi,#x
	mov	eax,[ebp*8+ytab_bank]
	cmp	eax,[edx].wwrcurbank
	jne	short #checksplit
#nosplit:
	add	edi,[edx].wwraddr
	add	edi,[ebp*8+ytab_offs]
#packetloop:
	dec	ebx
	js	short #lineloop
	lodsw
	mov	cl,al
	add	edi,ecx
	mov	cl,ah
	test	cl,cl
	js	short #dorun
	shr	ecx,2
	rep movsd
	and	ah,3
	mov	cl,ah
	rep movsb
	jmp	short #packetloop

	align 4
#dorun:
	neg	cl
	lodsb
	mov	ah,al
	shr	ecx,1
	rep stosw
	adc	ecx,0
	rep stosb
	jmp	short #packetloop

	align 4
#checksplit:
	test	eax,0FFFF0000h		; split line or just bank swap?
	jnz	short #splitline	; split lines get special handling.
	SetWriteBank			; if not split, just change to the
	jmp	short #nosplit		; new bank, then continue as normal.


	align 4
#splitdone:
	xor	ecx,ecx
	lea	edx,pj_vdrv_wcontrol
	jmp	short #lineloop

	align 4
#splitline:
	dec	ebx
	js	short #splitdone
	lodsw
	mov	cl,al
	add	edi,ecx
	mov	cl,ah
	test	cl,cl
	js	short #splitrun

	push	ecx
	push	ebp
	push	edi
	push	esi
	push	0
	add	esi,ecx
	add	edi,ecx
	call	pj_vdrv_put_hseg
	add	esp,20
	jmp	short #splitline

	align 4
#splitrun:
	neg	cl
	lodsb
	push	ecx
	push	ebp
	push	edi
	push	eax
	push	0
	add	edi,ecx
	call	pj_vdrv_set_hline
	add	esp,20
	jmp	short #splitline

pj_vdrv_unlccomp_rect endp

_TEXT	ends
	end


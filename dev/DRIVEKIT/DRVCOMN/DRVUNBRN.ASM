;*****************************************************************************
;* DRVUNB.ASM - The unbrun_rect routine for the vesa driver.
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

	public	pj_vdrv_unbrun_rect

;*****************************************************************************
;* void pj_vdrv_unbrun_rect(Raster *r, void *ucbuf, LONG pixsize,
;*			 Coor x, Coor y, Ucoor width, Ucoor height);
;*****************************************************************************

LONGRUN equ	20h			; threshold of a 'long' run.

pj_vdrv_unbrun_rect proc near

	Entry
	Args	#raster,#ucbuf,#pixsize,#x,#y,#w,#h
	Save	ebx,esi,edi,ebp,es

	mov	ax,gs
	mov	es,ax

	mov	esi,#ucbuf
	xor	ecx,ecx
	mov	ebp,#y
	xor	edx,edx
	jmp	short #lineloop
#done:
	Restore ebx,esi,edi,ebp,es
	Exit

	align 4
#checksplit:
	test	eax,0FFFF0000h		; split line or just bank swap?
	jnz	#splitline		; split lines get special handling.
	lea	edx,pj_vdrv_wcontrol		; load pj_vdrv_wcontrol ptr for bank routines
	SetWriteBank			; if not split, just change to the
	jmp	short #nosplit		; new bank, then continue as normal.

	align 4
#lineloop:
	dec	dptr #h
	js	short #done
	mov	edi,#x			; reload starting X
	mov	eax,[ebp*8+ytab_bank]
	mov	ebx,#w			; reload output width
	inc	esi			; skip crufty packet count
	cmp	eax,pj_vdrv_wcontrol.wwrcurbank
	jne	short #checksplit
#nosplit:
	add	edi,[ebp*8+ytab_offs]
	inc	ebp			; done with Y, incr for next time
	add	edi,pj_vdrv_wcontrol.wwraddr
	align 4
#packetloop:
	test	ebx,ebx
	jz	short #lineloop
	mov	cl,[esi]
	test	cl,cl
	js	short #doliteral

	sub	ebx,ecx 		; subtract width of this run from the
	mov	al,[esi+1]		; remaining line width (loop control).
	add	esi,2			; increment input pointer.

	cmp	cl,LONGRUN		; if it's a short run, go do it. else
	jb	short #shortrun 	; see if we can combine this run with
	xor	edx,edx
#combineruns:				; the next one.
	test	ebx,ebx
	jz	short #dolongrun
	mov	dl,[esi]		;
	test	dl,dl			;
	js	short #dolongrun	;
	cmp	al,[esi+1]		;
	jne	short #dolongrun	;
	add	esi,2			;
	add	ecx,edx 		;
	sub	ebx,edx 		;
	jmp	short #combineruns

	align 4
#dolongrun:
	mov	dl,al
	mov	dh,dl
	shrd	eax,edx,16
	mov	ax,dx
	mov	dl,cl
	and	dl,3
	shr	ecx,2
	rep stosd
	mov	cl,dl
	rep stosb
	jmp	short #packetloop

	align 4
#shortrun:
	rep stosb			; short run: just store the run,
	jmp	short #packetloop	; and continue with next packet.

	align 4
#doliteral:
	inc	esi
	neg	cl
	sub	ebx,ecx
	mov	al,cl
	and	al,3
	shr	ecx,2
	rep movsd
	mov	cl,al
	rep movsb
	jmp	short #packetloop


#splitdone:
	xor	ecx,ecx
	xor	edx,edx
	inc	ebp
	jmp	#lineloop

	align 4
#splitline:

	test	ebx,ebx
	jz	short #splitdone
	movsx	ecx,bptr [esi]
	inc	esi
	test	ecx,ecx
	js	short #splitliteral

	lodsb
	push	ecx
	push	ebp
	push	edi
	push	eax
	push	0
	sub	ebx,ecx
	add	edi,ecx
	call	pj_vdrv_set_hline
	add	esp,20
	jmp	short #splitline

	align 4
#splitliteral:
	neg	ecx
	push	ecx
	push	ebp
	push	edi
	push	esi
	push	0
	sub	ebx,ecx
	add	edi,ecx
	add	esi,ecx
	call	pj_vdrv_put_hseg
	add	esp,20
	jmp	short #splitline

pj_vdrv_unbrun_rect endp

_TEXT	ends
	end



	include stdmacro.i
	include raster.i

_TEXT	segment

	public pj_mcga_get_hseg
	public pj_mcga_put_hseg

;*****************************************************************************
;* LONG pj_mcga_get_hseg(Bytemap *r,void *pixbuf, ULONG x,ULONG y,ULONG width);
;* unclipped! get horizontal line segment
;*****************************************************************************

	align 4
pj_mcga_get_hseg proc near

	Entry
	Args	#raster,#pixbuf,#x,#y,#width
	Save	esi,edi

	mov	esi,#raster		; load raster pointer
	mov	edi,#pixbuf		; load output buffer pointer
	mov	ecx,#width		; load width
	mov	eax,#y			; standard calc-the-screen-address...
	mul	[esi].bym_bpr
	add	eax,#x
	mov	esi,[esi].bym_p
	add	esi,eax

	mov	eax,ecx 		; save width as return value in eax

	shr	ecx,2			; standard fast-move...
	rep movs dptr es:[edi],gs:[esi]
	mov	cl,al
	and	cl,3
	rep movs bptr es:[edi],gs:[esi]

	Restore esi,edi
	Exit

pj_mcga_get_hseg endp

;*****************************************************************************
;* LONG pj_mcga_put_hseg(Bytemap *r,void *pixbuf, ULONG x,ULONG y,ULONG width);
;* unclipped! put horizontal line segment
;*****************************************************************************

	align 4
pj_mcga_put_hseg proc near

	Entry
	Args	#raster,#pixbuf,#x,#y,#width
	Save	esi,edi,es

	mov	edi,#raster		; load raster pointer
	mov	esi,#pixbuf		; load input buffer pointer
	mov	ecx,#width		; load width
	mov	eax,#y			; standard calc-the-screen-address...
	mul	[edi].bym_bpr
	add	eax,#x
	mov	edi,[edi].bym_p
	add	edi,eax

	mov	ax,gs			; make screen addressable via ES reg
	mov	es,ax			; for rep movs instructions.

	mov	eax,ecx 		; save width as return value in eax

	shr	ecx,2			; standard fast-move...
	rep movsd
	mov	cl,al
	and	cl,3
	rep movsb

	Restore esi,edi,es
	Exit

pj_mcga_put_hseg endp

_TEXT	ends
	end

;*****************************************************************************
;* MCGADOT.ASM - Get and put dot routines for MCGA builtin driver.
;*
;* NOTES:
;*	The code below uses IMUL instead MUL in a couple places, because the
;*	latter trashes the EDX register while the former does not.  Don't
;*	change these unless you do something to preserve edx in context.
;*
;*****************************************************************************

	include stdmacro.i
	include raster.i

_TEXT	segment

	public pj_mcga_cget_dot
	public pj_mcga_get_dot
	public pj_mcga_cput_dot
	public pj_mcga_put_dot

;*****************************************************************************
;* int pj_mcga_cget_dot(Vscreen *v, int x, int y);
;* int pj_mcga_get_dot(Vscreen *v, int x, int y);
;*****************************************************************************

	align 4
pj_mcga_cget_dot proc near

	Entry
	Args	#raster,#x,#y

	mov	edx,#raster		; load raster pointer

	mov	ecx,#x			; load X coordinate, compare it to max
	cmp	cx,[edx].bym_w		; X, if greater, we clip.  note jae
	jae	short #clipped		; makes negative X appear larger.

	mov	eax,#y			; load Y coordinate, compare it to max
	cmp	ax,[edx].bym_h		; Y, if greater, we clip.  note jb
	jb	short do_getdot 	; makes negative Y appear larger.

#clipped:

	xor	eax,eax 		; clipped, return color 0.
	Exit

pj_mcga_cget_dot endp

	align 4
pj_mcga_get_dot proc near

	Entry
	Args	#raster,#x,#y

	mov	edx,#raster		; get raster pointer
	mov	ecx,#x			; load X coordinate
	mov	eax,#y			; get Y coordinate
do_getdot:
	imul	eax,[edx].bym_bpr	; make offset to start of line
	add	eax,ecx 		; add X coordinate
	add	eax,[edx].bym_p 	; add base address of screen
	movzx	eax,bptr gs:[eax]	; get the byte, zero-extended

	Exit

pj_mcga_get_dot endp

;*****************************************************************************
;* void pj_mcga_cput_dot(Bytemap *v, Pixel color, LONG x, LONG y);
;* void pj_mcga_put_dot(Bytemap *v, Pixel color, LONG x, LONG y);
;*****************************************************************************

	align 4
pj_mcga_cput_dot proc near

	Entry
	Args	#raster,#color,#x,#y

	mov	edx,#raster		; load raster pointer

	mov	ecx,#x			; load X coordinate, compare it to max
	cmp	cx,[edx].bym_w		; X, if greater, we clip.  note jae
	jae	short #clipped		; makes negative X appear larger.

	mov	eax,#y			; load Y coordinate, compare it to max
	cmp	ax,[edx].bym_h		; Y, if greater, we clip.  note jb
	jb	short do_putdot 	; makes negative Y appear larger.

#clipped:

	Exit

pj_mcga_cput_dot endp

	align 4
pj_mcga_put_dot proc near

	Entry
	Args	#raster,#color,#x,#y

	mov	edx,#raster		; load raster pointer
	mov	ecx,#x			; load X coordinate
	mov	eax,#y			; load Y coordinate
do_putdot:
	imul	eax,[edx].bym_bpr	; make offset to start of line
	add	eax,ecx 		; add X coordinate
	add	eax,[edx].bym_p 	; add based address of screen
	mov	edx,#color		; load output pixel
	mov	gs:[eax],dl		; store it to screen

	Exit

pj_mcga_put_dot endp

_TEXT	ends
	end


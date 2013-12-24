;*****************************************************************************
;* HGSDOTS.ASM - Get/Put dot routines for PJ HGSC driver.
;*****************************************************************************
	include hgsc.inc
	include macro.inc

_CODE	segment para public use32 'CODE'

	public	hgs_putdot
	public	hgs_cputdot
	public	hgs_getdot
	public	hgs_cgetdot

;---------------------------------------------------------------------------
; void hgs_putdot(Hrast *r, Pixel color, Coor x, Coor y);
;---------------------------------------------------------------------------

hgs_cputdot proc near

	mov	eax,[esp+4]		; load pointer to Hrast structure
	mov	edx,[esp+16]		; load Y coordinate
	cmp	dx,[eax].rast_height	; compare Y against raster height
	jae	short ppunt		; if out of range, punt
	mov	ecx,[esp+12]		; load X coordinate
	cmp	cx,[eax].rast_width	; compare X against raster width
	jb	pdo_it			; if in range, go do it
ppunt:
	ret				; out of range, punt.

hgs_putdot proc near

	mov	edx,[esp+16]		; load Y coordinate
	mov	ecx,[esp+12]		; load X coordinate
pdo_it:
	push	ds			; establish addressibility
	mov	ax,SS_DOSMEM		; to the HGSC registers
	mov	ds,ax			; and VGA memory segment.

	mov	wpds:[HCTRL],0C000h	; halt 34010, no incw, no incr

	shl	edx,YSHFTMUL		; scale Y to offset in hgs memory
	test	ecx,1			; odd X address?
	jz	short pnotodd		; nope, go do even address
	lea	edx,[ecx*8+edx-8]	; make eax pointer to word in hgs mem
	mov	ds:[HADDL],dx		; set host addr low
	shr	edx,16
	mov	ds:[HADDH],dx		; set host addr high
	mov	ax,ds:[HDATA16] 	; get word
	mov	ah,[esp+12]		; put in new pixel
	mov	ds:[HDATA16],ax 	; put word
	pop	ds
	ret
pnotodd:
	lea	edx,[ecx*8+edx] 	; make eax pointer to word in hgs mem
	mov	ds:[HADDL],dx		; set host addr low
	shr	edx,16
	mov	ds:[HADDH],dx		; set host addr high
	mov	ax,ds:[HDATA16] 	; get word
	mov	al,[esp+12]		; put in new pixel
	mov	ds:[HDATA16],ax 	; put word
	pop	ds
	ret

hgs_putdot endp
hgs_cputdot endp

;---------------------------------------------------------------------------
; hgs_getdot
;	Pixel hgs_getdot(Hrast *r, Coor x, Coor y);
;---------------------------------------------------------------------------

hgs_cgetdot proc near

	mov	eax,[esp+4]		; load pointer to Hrast structure
	mov	edx,[esp+12]		; load Y coordinate
	cmp	dx,[eax].rast_height	; compare Y against raster height
	jae	short gpunt		; if out of range, punt
	mov	ecx,[esp+8]		; load X coordinate
	cmp	cx,[eax].rast_width	; compare X against raster width
	jb	gdo_it			; if in range, go do it
gpunt:
	mov	eax,0			; dot out of range, return zero
	ret				; without further processing.

hgs_getdot proc near

	mov	edx,[esp+12]		; load Y coordinate
	mov	ecx,[esp+8]		; load X coordinate
gdo_it:
	push	ds

	mov	ax,SS_DOSMEM
	mov	ds,ax

	mov	wpds:[HCTRL],0C000h	; halt 34010, no incw, no incr

	shl	edx,YSHFTMUL		; scale to offset in hgs memory
	test	ecx,1			; odd address?
	jz	short gnotodd		; nope, go do even address
	lea	eax,[ecx*8+edx-8]	; make eax pointer to word in hgs mem
	mov	ds:[HADDL],ax		; set host addr low
	shr	eax,16
	mov	ds:[HADDH],ax		; set host addr high
	movzx	eax,wpds:[HDATA16]	; get word
	shr	eax,8			; move pixel to low-order
	pop	ds
	ret
gnotodd:
	lea	eax,[ecx*8+edx] 	; make eax pointer to word in hgs mem
	mov	ds:[HADDL],ax		; set host addr low
	shr	eax,16
	mov	ds:[HADDH],ax		; set host addr high
	mov	ax,wpds:[HDATA16]	; get word
	and	eax,0FFh		; mask off high order, pixel in low
	pop	ds
	ret

hgs_getdot endp
hgs_cgetdot endp

_CODE	ends
	end


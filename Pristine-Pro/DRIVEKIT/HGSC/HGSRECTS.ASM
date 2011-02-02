;*****************************************************************************
;* HGSRECTS.ASM - Rectangle-related routines for PJ HGSC SVGA driver.
;*****************************************************************************
	include hgsc.inc
	include macro.inc

_CODE	segment public para use32 'CODE'

	public	hgs_put_rectpix
	public	hgs_get_rectpix
	public	hgs_setrect
	public	hgs_setrast

theraster equ	[ebp+8]
color	equ	[ebp+12]
buffer	equ	[ebp+12]
xcoor	equ	[ebp+16]
ycoor	equ	[ebp+20]
wcoor	equ	[ebp+24]
hcoor	equ	[ebp+28]

;----------------------------------------------------------------------------
; void hgs_put_rectpix(Hrast *r, Pixel *buffer, Coor x,y,w,h);
;----------------------------------------------------------------------------

hgs_put_rectpix proc near

	Entry
	Save	ebx,esi,edi,es

	mov	ax,SS_DOSMEM
	mov	es,ax

	mov	wpes:[HCTRL],0C800h	; halt, incw, no incr

	mov	esi,buffer		; load source buffer pointer
	mov	ebx,hcoor		; load rectangle height
	mov	edx,ycoor		; load rectangle y coordinate
	shl	edx,YSHFTMUL		; multiply to get hgsc memory offset
	mov	eax,xcoor		; load rectangle x coordinate
	test	eax,1			; does rectangle start on an odd pixel?
	jz	pr_doeven		; nope, go do (fast) even-aligned loop

	lea	edx,[eax*8+edx-8]	; make eax pointer to word in hgs mem
	push	ebp
	mov	ebp,wcoor
	dec	ebp			; one less pixel to do in loop
pr_oddloop:
	mov	ecx,ebp 		; load segment length
	call	_hgs_put_ohseg		; go put odd-boundry segment
	add	edx,YNEXTLINE		; point to next line in hgsc memory
	sub	ebx,1			; decrement line counter
	jnz	pr_oddloop		; if not zero, go do it some more
	jmp	short pr_done		; all done

pr_doeven:
	lea	edx,[eax*8+edx] 	; make edx pointer to word in hgs
	push	ebp
	mov	ebp,wcoor
pr_evenloop:
	mov	ecx,ebp 		; load segment length
	call	_hgs_put_ehseg		; go put even-boundry segment
	add	edx,YNEXTLINE		; point to next line in hgsc memory
	sub	ebx,1			; decrement line counter
	jnz	pr_evenloop		; if not zero, go do it some more

pr_done:
	pop	ebp
	Restore ebx,esi,edi,es
	Exit

hgs_put_rectpix endp

;----------------------------------------------------------------------------
; void hgs_get_rectpix(Hrast *r, Pixel *buffer, Coor x,y,w,h);
;----------------------------------------------------------------------------

hgs_get_rectpix proc near

	Entry
	Save	ebx,esi,edi,ds

	mov	ax,SS_DOSMEM
	mov	ds,ax

	mov	wpds:[HCTRL],0D000h	; halt, incr, no incw

	mov	edi,buffer		; load source buffer pointer
	mov	ebx,hcoor		; load rectangle height
	mov	edx,ycoor		; load rectangle y coordinate
	shl	edx,YSHFTMUL		; multiply to get hgsc memory offset
	mov	eax,xcoor		; load rectangle x coordinate
	test	eax,1			; does rectangle start on an odd pixel?
	jz	gr_doeven		; nope, go do (fast) even-aligned loop

	lea	edx,[eax*8+edx-8]	; make edx pointer to word in hgs mem
	push	ebp
	mov	ebp,wcoor
	dec	ebp			; one less pixel to do in loop
gr_oddloop:
	mov	ecx,ebp 		; get width parameter
	call	_hgs_get_ohseg		; call move loop
	add	edx,YNEXTLINE		; point to next line in hgsc memory
	sub	ebx,1			; decrement line counter
	jnz	gr_oddloop		; if not zero, go do it some more
	jmp	short gr_done		; all done

gr_doeven:
	lea	edx,[eax*8+edx] 	; make edx pointer to word in hgs
	push	ebp
	mov	ebp,wcoor
gr_evenloop:
	mov	ecx,ebp 		; get width parameter
	call	_hgs_get_ehseg		; call move loop
	add	edx,YNEXTLINE		; point to next line in hgsc memory
	sub	ebx,1			; decrement line counter
	jnz	gr_evenloop		; if not zero, go do it some more

gr_done:
	pop	ebp
	Restore ebx,esi,edi,ds
	Exit

hgs_get_rectpix endp

;----------------------------------------------------------------------------
; void hgs_setrect(Hrast *r, Pixel color, Coor x,y,w,h);
;----------------------------------------------------------------------------

hgs_setrect proc near

	Entry
	Save	ebx,esi,edi,es

	mov	ax,SS_DOSMEM
	mov	es,ax

	mov	wpes:[HCTRL],0C800h	; halt, incw, no incr

	mov	ebx,hcoor		; load rectangle height
	mov	edx,ycoor		; load rectangle y coordinate
	shl	edx,YSHFTMUL		; multiply to get hgsc memory offset
	mov	eax,xcoor		; load rectangle x coordinate
	test	eax,1			; does rectangle start on an odd pixel?
	jz	sr_doeven		; nope, go do (fast) even-aligned loop

	dec	eax			; odd address, make it even
	dec	dword ptr wcoor 	; one less pixel to do in loop
	lea	edx,[eax*8+edx] 	; make eax pointer to word in hgs mem
sr_oddloop:
	mov	edi,HDATA16		; point to fast hgsc memory
	mov	eax,edx 		; load current line offset
	mov	es:[HADDL],ax		; set host addr low
	shr	eax,16
	mov	es:[HADDH],ax		; set host addr high
	mov	ax,es:[HDATA16] 	; get word from scanline
	mov	ah,color		; lay in odd pixel
	stosw				; save word to scanline
	mov	al,ah			; fill in other half of constant reg
	shl	eax,8
	mov	al,ah
	shl	eax,8
	mov	al,ah
	mov	ecx,wcoor		; get width parameter
	mov	esi,ecx 		; save width for endcase handling
	shr	ecx,2			; we move words, adjust count
	rep stosd			; move the pixels
	jnc	sr_noword1
	stosw
sr_noword1:
	test	esi,1			; was the count odd?
	jz	short sr_oddend 	; nope, skip the endcase handling
	lea	esi,[esi*8+edx+8]	;
	mov	wpes:[HADDL],si
	shr	esi,16
	mov	wpes:[HADDH],si
	mov	ax,es:[HDATA16] 	; get the pixel pair from screen
	mov	al,color		; lay in odd pixel
	stosw				; store new pixel pair
sr_oddend:
	add	edx,YNEXTLINE		; point to next line in hgsc memory
	sub	ebx,1			; decrement line counter
	jnz	sr_oddloop		; if not zero, go do it some more
	jmp	short sr_done		; all done

sr_doeven:
	lea	edx,[eax*8+edx] 	; make edx pointer to word in hgs
	mov	al,color
	mov	ah,al
	shl	eax,8
	mov	al,ah
	shl	eax,8
	mov	al,ah
sr_evenloop:
	mov	edi,HDATA16		; point to fast hgs memory
	mov	esi,edx 		; load current line offset
	mov	es:[HADDL],si		; set host addr low
	shr	esi,16
	mov	es:[HADDH],si		; set host addr high
	mov	ecx,wcoor		; get width parameter
	mov	esi,ecx 		; save width for endcase handling
	shr	ecx,2			; we move words, adjust count
	rep stosd			; move the pixels
	jnc	sr_noword2
	stosw
sr_noword2:
	test	esi,1			; was the count odd?
	jz	short sr_evenend	; nope, skip the endcase handling
	lea	esi,[esi*8+edx] 	; yep, point 34010 to word where
	mov	wpes:[HADDL],si 	; we'll lay in the last pixel
	shr	esi,16
	mov	wpes:[HADDH],si
	mov	ax,es:[HDATA16] 	; get the pixel pair from screen
	mov	al,color		; lay in odd pixel
	stosw				; store new pixel pair
	mov	ah,al			; restore upper part of color constant
sr_evenend:
	add	edx,YNEXTLINE		; point to next line in hgsc memory
	sub	ebx,1			; decrement line counter
	jnz	sr_evenloop		; if not zero, go do it some more

sr_done:
	Restore ebx,esi,edi,es
	Exit

hgs_setrect endp

;----------------------------------------------------------------------------
; void hgs_setrast(Hrast *theraster, Pixel color);
;----------------------------------------------------------------------------

hgs_setrast proc  near

	Entry
	Save	ebx,esi,edi,es

	mov	ax,SS_DOSMEM
	mov	es,ax

	mov	wpes:[HCTRL],0C800h	; halt 34010, incw, no incr

	mov	eax,theraster
	movzx	ebx,[eax].rast_height
	movzx	edx,[eax].rast_width
	shr	edx,2
	xor	esi,esi

	mov	al,color
	mov	ah,al
	mov	cx,ax
	shl	eax,16
	mov	ax,cx
rastloop:
	mov	ecx,esi
	add	esi,YNEXTLINE
	mov	wpes:[HADDL],cx
	shr	ecx,16
	mov	wpes:[HADDH],cx
	mov	edi,HDATA16
	mov	ecx,edx
	rep stosd
	sub	ebx,1
	jnz	rastloop

	Restore ebx,esi,edi,es
	Exit

hgs_setrast endp

_CODE	ends
	end

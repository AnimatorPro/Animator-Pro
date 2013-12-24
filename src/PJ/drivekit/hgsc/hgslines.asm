;*****************************************************************************
;* HGSLINES.ASM - Set horizontal/vertical lines for PJ HGSC driver.
;*****************************************************************************

	include hgsc.inc
	include macro.inc

_CODE	segment public para use32 'CODE'

	public	hgs_set_hline
	public	hgs_set_vline

color	 equ	 [ebp+12]
xcoor	 equ	 [ebp+16]
ycoor	 equ	 [ebp+20]
pixcount equ	 [ebp+24]

;----------------------------------------------------------------------------
; void hgs_set_hline(Hrast *r, Pixel color, Coor x, Coor y, Ucoor w);
;----------------------------------------------------------------------------

hgs_set_hline proc	near

	Entry
	Save	edi,es

	mov	ax,SS_DOSMEM
	mov	es,ax
	mov	edi,HDATA16

	mov	wpes:[HCTRL],0C800h	; halt 34010, incw, no incr

	mov	ecx,pixcount		; get width
	mov	eax,ycoor		; get y
	shl	eax,YSHFTMUL		; scale to offset in hgs memory
	mov	edx,xcoor		; get x
	test	edx,1			; odd address?
	jz	short hnotodd		; nope, go do even address

	dec	ecx			; one less pixel to do in loop
	lea	eax,[edx*8+eax-8]	; make pointer to line in hgs mem
	lea	edx,[ecx*8+eax+8]	; make pointer for handling odd endcase
	mov	es:[HADDL],ax		; set host addr low
	shr	eax,16
	mov	es:[HADDH],ax		; set host addr high
	mov	ax,es:[HDATA16] 	; get word
	mov	ah,color		; put in new pixel
	mov	es:[HDATA16],ax 	; put word
	jmp	short hdorun
hnotodd:
	lea	eax,[edx*8+eax]
	lea	edx,[ecx*8+eax]
	mov	es:[HADDL],ax		; set host addr low
	shr	eax,16
	mov	es:[HADDH],ax		; set host addr high
hdorun:
	mov	al,color
	mov	ah,al
	shl	eax,8
	mov	al,ah
	shl	eax,8
	mov	al,ah
	shr	ecx,1
	jc	short hoddrun
	shr	ecx,1
	rep stosd
	jnc	heven_noword
	stosw
heven_noword:
	Restore edi,es
	Exit
hoddrun:
	shr	ecx,1
	rep stosd
	jnc	hodd_noword
	stosw
hodd_noword:
	mov	wpes:[HADDL],dx
	shr	edx,16
	mov	wpes:[HADDH],dx
	mov	dx,es:[HDATA16]
	mov	dl,al
	mov	es:[HDATA16],dx
	Restore edi,es
	Exit

hgs_set_hline endp

;----------------------------------------------------------------------------
; void hgs_set_vline(Hrast *r, Pixel color, Coor x, Coor y, Ucoor h);
;----------------------------------------------------------------------------

hgs_set_vline proc	near

	Entry
	Save	ebx,es

	mov	ax,SS_DOSMEM
	mov	es,ax

	mov	wpes:[HCTRL],0C000h	; halt 34010, no incw, no incr

	mov	ecx,pixcount		; load line height
	mov	edx,ycoor		; load y coordinate
	shl	edx,YSHFTMUL		; scale to hgs memory
	mov	ebx,xcoor		; load x coordinate
	test	ebx,1			; is x odd?
	jz	short vnotodd		; nope, go to even loop

	lea	edx,[ebx*8+edx-8]	; make pointer to hgs memory
	mov	ebx,color		; load pixel value
voddloop:
	mov	eax,edx 		; load current hgs memory pointer
	mov	wpes:[HADDL],ax 	; point hgs to current location
	shr	eax,16
	mov	wpes:[HADDH],ax
	mov	ax,wpes:[HDATA16]	; get pixel pair
	mov	ah,bl			; lay in new pixel value
	mov	wpes:[HDATA16],ax	; write new pixel pair
	add	edx,YNEXTLINE		; point to next scanline in hgs memory
	loop	voddloop		; do it some more
	jmp	short vdone		; all done

vnotodd:
	lea	edx,[ebx*8+edx] 	; make pointer to hgs memory
	mov	ebx,color		; load pixel value
vevenloop:
	mov	eax,edx 		; load current hgs memory pointer
	mov	wpes:[HADDL],ax 	; point hgs to current location
	shr	eax,16
	mov	wpes:[HADDH],ax
	mov	ax,wpes:[HDATA16]	; get pixel pair
	mov	al,bl			; lay in new pixel value
	mov	wpes:[HDATA16],ax	; write new pixel pair
	add	edx,YNEXTLINE		; point to next scanline in hgs memory
	loop	vevenloop		; do it some more
vdone:
	Restore ebx,es
	Exit

hgs_set_vline endp

_CODE	ends
	end

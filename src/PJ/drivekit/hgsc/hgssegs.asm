;*****************************************************************************
;* HGSSEGS.ASM - Get/Put horizontal and vertical segments for PJ HGSC driver.
;*****************************************************************************

	include hgsc.inc
	include macro.inc

_CODE	segment public para use32 'CODE'

	public	_hgs_get_ohseg		; these are shortcut (parms in regs)
	public	_hgs_get_ehseg		; entry points for other use by
	public	_hgs_put_ohseg		; other assembler routines.
	public	_hgs_put_ehseg

	public	hgs_get_hseg		; these are the routines called by PJ.
	public	hgs_put_hseg
	public	hgs_put_vseg
	public	hgs_get_vseg

pbuf	 equ	 [ebp+12]
xcoor	 equ	 [ebp+16]
ycoor	 equ	 [ebp+20]
pixcount equ	 [ebp+24]

;----------------------------------------------------------------------------
; _hgs_get_ohseg - internal service routine to get a horizontal segment.
; _hgs_get_ehseg - internal service routine to get a horizontal segment.
;
;  ohseg is tuned for an odd X coordinate
;  ehseg is tuned for an even X coordinate
;
;   Entry:
;     edx - pointer into hgs memory
;     ecx - width
;     edi - destination buffer
;      ds - addresses SS_DOSMEM (offset zero == abosolute zero, 0x0034)
;      es - addresses SS_DATA	(normal pharlap data segment 0x0014)
;   Exit:
;     eax - trashed
;     ecx - zeroed
;     esi - trashed
;     edi - pointer to next byte in destination buffer
;     all others preserved
;----------------------------------------------------------------------------

_hgs_get_ohseg proc near
	mov	esi,HDATA16		; point to fast hgsc memory
	mov	eax,edx 		; load current line offset
	mov	ds:[HADDL],ax		; set host addr low
	shr	eax,16
	mov	ds:[HADDH],ax		; set host addr high
	mov	ax,ds:[HDATA16] 	; get word from scanline
	mov	al,ah			; move pixel to low order,
	stosb				; store it in output buffer
	shr	ecx,1			; we move words, adjust count
	jc	gho_oddlength		; if length is odd, do odd loop.
	shr	ecx,1
	rep movsd			; move the pixels
	jnc	short gho_noword1
	movsw
gho_noword1:
	ret
gho_oddlength:
	shr	ecx,1
	rep movsd			; move the pixels
	jnc	short gho_noword2
	movsw
gho_noword2:
	mov	ax,ds:[HDATA16] 	; get next pixel from screen
	stosb				; store it in output buffer
	ret
_hgs_get_ohseg endp

_hgs_get_ehseg proc near
	mov	esi,HDATA16		; point to fast hgs memory
	mov	eax,edx 		; load current line offset
	mov	ds:[HADDL],ax		; set host addr low
	shr	eax,16
	mov	ds:[HADDH],ax		; set host addr high
	shr	ecx,1			; we move words, adjust count
	jc	ghe_oddlength		; if length is odd, do odd loop.
	shr	ecx,1
	rep movsd			; move the pixels
	jnc	short ghe_noword1
	movsw
ghe_noword1:
	ret
ghe_oddlength:
	shr	ecx,1
	rep movsd			; move the pixels
	jnc	short ghe_noword2
	movsw
ghe_noword2:
	mov	ax,ds:[HDATA16] 	; get next pixel from screen
	stosb				; store it in output buffer
	ret
_hgs_get_ehseg endp

;----------------------------------------------------------------------------
; _hgs_put_ohseg - internal service routine to put a horizontal segment.
; _hgs_put_ehseg - internal service routine to put a horizontal segment.
;
;  ohseg is tuned for an odd X coordinate
;  ehseg is tuned for an even X coordinate
;
;   Entry:
;     ecx - width
;     edx - pointer into hgs memory
;     esi - source buffer
;      ds - addresses SS_DATA	(normal pharlap data segment 0x0014)
;      es - addresses SS_DOSMEM (offset zero == abosolute zero, 0x0034)
;   Exit:
;     eax - trashed
;     ecx - zeroed
;     esi - pointer to next byte in source buffer
;     edi - trashed
;     all others preserved
;----------------------------------------------------------------------------

_hgs_put_ohseg proc near
	mov	edi,HDATA16		; point to fast hgsc memory
	mov	eax,edx 		; load current line offset
	mov	es:[HADDL],ax		; set host addr low
	shr	eax,16
	mov	es:[HADDH],ax		; set host addr high
	mov	ax,es:[HDATA16] 	; get word from scanline
	mov	ah,[esi]		; put in next byte from buffer
	mov	es:[HDATA16],ax 	; save word to scanline
	inc	esi			; increment bufptr
	mov	eax,ecx 		; save width for endcase handling
	shr	ecx,2			; we move words, adjust count
	rep movsd			; move the pixels
	jnc	pho_noword1
	movsw
pho_noword1:
	test	eax,1			; was the count odd?
	jz	short ph_oddend 	; nope, skip the endcase handling
	lea	eax,[eax*8+edx+8]	;
	mov	wpes:[HADDL],ax
	shr	eax,16
	mov	wpes:[HADDH],ax
	mov	ax,es:[HDATA16] 	; get the pixel pair from screen
	lodsb				; lay in next pixel from data buffer
	stosw				; store new pixel pair
ph_oddend:
	ret
_hgs_put_ohseg endp

_hgs_put_ehseg proc near
	mov	edi,HDATA16		; point to fast hgs memory
	mov	eax,edx 		; load current line offset
	mov	es:[HADDL],ax		; set host addr low
	shr	eax,16
	mov	es:[HADDH],ax		; set host addr high
	mov	eax,ecx 		; save width for endcase handling
	shr	ecx,2			; we move words, adjust count
	rep movsd			; move the pixels
	jnc	phe_noword1
	movsw
phe_noword1:
	test	eax,1			; was the count odd?
	jz	short ph_evenend	; nope, skip the endcase handling
	lea	eax,[eax*8+edx] 	; yep, point 34010 to word where
	mov	wpes:[HADDL],ax 	; we'll lay in the last pixel
	shr	eax,16
	mov	wpes:[HADDH],ax
	mov	ax,es:[HDATA16] 	; get the pixel pair from screen
	lodsb				; lay in next pixel from data buffer
	stosw				; store new pixel pair
ph_evenend:
	ret
_hgs_put_ehseg endp

;----------------------------------------------------------------------------
; void hgs_get_hseg(Hrast *r, Pixel *pbuf, Coor x, Coor y, Ucoor w);
;----------------------------------------------------------------------------

hgs_get_hseg proc  near

	Entry
	Save	esi,edi,ds

	mov	ax,SS_DOSMEM
	mov	ds,ax

	mov	wpds:[HCTRL],0D000h	; halt, incr, no incw

	mov	edi,pbuf		; load pointer to output buffer
	mov	ecx,pixcount		; load segment width
	mov	edx,ycoor		; load Y coordinate
	shl	edx,YSHFTMUL		; scale Y to offset in hgs memory
	mov	eax,xcoor		; load X coordinate
	test	eax,1			; is X on an odd boundry?
	jz	short gh_doeven 	; nope, go do even transfer

	dec	ecx			; odd pixel handled separately, one
	lea	edx,[eax*8+edx-8]	; less to do in loop, make pointer to
	call	_hgs_get_ohseg		; hgs memory, go do the move loop
	jmp	short gh_done		; all done
gh_doeven:
	lea	edx,[eax*8+edx] 	; make pointer to hgs memory
	call	_hgs_get_ehseg		; go do the move loop
gh_done:
	Restore esi,edi,ds
	Exit

hgs_get_hseg endp

;----------------------------------------------------------------------------
; void hgs_put_hseg(Hrast *r, Pixel *pbuf, Coor x, Coor y, Ucoor w);
;----------------------------------------------------------------------------

hgs_put_hseg proc  near

	Entry
	Save	esi,edi,es

	mov	ax,SS_DOSMEM
	mov	es,ax

	mov	wpes:[HCTRL],0C800h	; halt 34010, no incr, incw

	mov	esi,pbuf		; get source buffer pointer
	mov	ecx,pixcount		; get width
	mov	edx,ycoor		; get y
	shl	edx,YSHFTMUL		; scale to offset in hgs memory
	mov	eax,xcoor		; get x
	test	eax,1			; is x odd?
	jz	short ph_doeven 	; nope, go do even

	dec	ecx			; odd pixel handled separately, one
	lea	edx,[eax*8+edx-8]	; less to do in loop, make pointer to
	call	_hgs_put_ohseg		; hgs memory, go do the move loop
	jmp	short ph_done		; all done
ph_doeven:
	lea	edx,[eax*8+edx] 	; make pointer to hgs memory
	call	_hgs_put_ehseg		; go do the move loop
ph_done:
	Restore esi,edi,es
	Exit

hgs_put_hseg endp

;----------------------------------------------------------------------------
; void hgs_put_vseg(Hrast *r, Pixel *pbuf, Coor x, Coor y, Ucoor h);
;----------------------------------------------------------------------------

hgs_put_vseg proc      near

	Entry
	Save	ebx,esi,es

	mov	ax,SS_DOSMEM
	mov	es,ax

	mov	wpes:[HCTRL],0C000h	; halt 34010, no incw, no incr

	mov	esi,pbuf		; load buffer pointer
	mov	ecx,pixcount		; load height
	mov	edx,ycoor		; load y coordinate
	shl	edx,YSHFTMUL		; scale to hgs memory
	mov	ebx,xcoor		; load x coordinate
	test	ebx,1			; is x odd?
	jz	short vpnotodd		; nope, go to even loop

	lea	edx,[ebx*8+edx-8]	; make pointer to hgs memory
vpoddloop:
	mov	eax,edx 		; load current hgs memory pointer
	mov	wpes:[HADDL],ax 	; point hgs to current location
	shr	eax,16
	mov	wpes:[HADDH],ax
	lodsb				; load next pixel from the buffer
	mov	bx,wpes:[HDATA16]	; get pixel pair from the screen
	mov	bh,al			; lay in new pixel value
	mov	wpes:[HDATA16],bx	; write new pixel pair
	add	edx,YNEXTLINE		; point to next scanline in hgs memory
	loop	vpoddloop		; do it some more
	jmp	short vpdone		; all done

vpnotodd:
	lea	edx,[ebx*8+edx] 	; make pointer to hgs memory
vpevenloop:
	mov	eax,edx 		; load current hgs memory pointer
	mov	wpes:[HADDL],ax 	; point hgs to current location
	shr	eax,16
	mov	wpes:[HADDH],ax
	lodsb				; load next pixel from the buffer
	mov	bx,wpes:[HDATA16]	; get pixel pair from the screen
	mov	bl,al			; lay in new pixel value
	mov	wpes:[HDATA16],bx	; write new pixel pair
	add	edx,YNEXTLINE		; point to next scanline in hgs memory
	loop	vpevenloop		; do it some more
vpdone:
	Restore ebx,esi,es
	Exit

hgs_put_vseg endp

;----------------------------------------------------------------------------
; void hgs_get_vseg(Hrast *r, Pixel *pbuf, Coor x, Coor y, Ucoor h);
;----------------------------------------------------------------------------

hgs_get_vseg proc      near

	Entry
	Save	edi,ds

	mov	ax,SS_DOSMEM
	mov	ds,ax

	mov	wpds:[HCTRL],0C000h	; halt 34010, no incw, no incr

	mov	edi,pbuf		; load buffer pointer
	mov	ecx,pixcount		; load height
	mov	edx,ycoor		; load y coordinate
	shl	edx,YSHFTMUL		; scale to hgs memory
	mov	eax,xcoor		; load x coordinate
	test	eax,1			; is x odd?
	jz	short vgnotodd		; nope, go to even loop

	lea	edx,[eax*8+edx-8]	; make pointer to hgs memory
vgoddloop:
	mov	eax,edx 		; load current hgs memory pointer
	mov	wpds:[HADDL],ax 	; point hgs to current location
	shr	eax,16
	mov	wpds:[HADDH],ax
	mov	ax,wpds:[HDATA16]	; get pixel pair from the screen
	mov	al,ah			; move the pixel to the output reg
	stosb				; store pixel into buffer
	add	edx,YNEXTLINE		; point to next scanline in hgs memory
	loop	vgoddloop		; do it some more
	jmp	short vgdone		; all done

vgnotodd:
	lea	edx,[eax*8+edx] 	; make pointer to hgs memory
vgevenloop:
	mov	eax,edx 		; load current hgs memory pointer
	mov	wpds:[HADDL],ax 	; point hgs to current location
	shr	eax,16
	mov	wpds:[HADDH],ax
	mov	ax,wpds:[HDATA16]	; get pixel pair from the screen
	stosb				; store pixel into output buffer
	add	edx,YNEXTLINE		; point to next scanline in hgs memory
	loop	vgevenloop		; do it some more
vgdone:
	Restore edi,ds
	Exit

hgs_get_vseg endp
_CODE	ends
	end

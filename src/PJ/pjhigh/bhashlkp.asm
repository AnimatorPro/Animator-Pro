;*****************************************************************************
;* BHASHLKP.ASM - the bclosest_col routine, cached color lookup w/dithering.
;*****************************************************************************

	include stdmacro.i
	assume	cs:CGROUP,ds:DGROUP

_BSS	segment

BhashCtl struc
cachedata  dd	?			; pointer to alloc'd cache data area
ctab	   dd	?			; contains vb.pencel->cmap->ctab
rederr	   dd	?			; error diffusion dithering variables...
grnerr	   dd	?
bluerr	   dd	?
drgb	   dd	?			; rgb value with dithering rolled in

;calls	    dd	 ?			 ; cache stats...
;hits1	    dd	 ?			 ;   to use these, you also need to
;hits2	    dd	 ?			 ;   uncomment a few lines below.
;fhits	    dd	 ?			 ;   search for 'bhashctl.' to find them.
;misses     dd	 ?

BhashCtl ends

	extrn	bhashctl:BhashCtl	; the one-and-only lives in bhash.c

_BSS	ends

_TEXT	segment

	extrn	closestc:near
	public	bclosest_col

;*****************************************************************************
;* int bclosest_col(Rgb3 *rgb, int count, SHORT dither)
;*
;*   this routine applies optional error diffusion dithering to the input
;*   rgb value, then looks up the color using a cached lookup.	if the color
;*   is not found in the cache, the closestc() routine is called, and the
;*   color it finds is stored into the cache.  if dithering is requested,
;*   the weighted difference between the requested and found colors is saved
;*   for use on the next call.	the cache tends to get hits in the 65-100%
;*   range, generally on the high end of that range.  (95% is typical except
;*   when dithering is used.  dithering can generate tens of thousands of
;*   unique colors, which sometimes flushes the cache too quickly.)
;*
;*   the dithering error from the last call is rolled into the rgb values of
;*   the current call using the following (conceptual) formula:
;*	  temp = rgb->r + rederr;
;*	  if (temp < 0)
;*	    temp = 0;
;*	  if (temp > 255)
;*	    temp = 255;
;*	  drgb.r = temp;
;*   the same logic is applied to each of the r,g,b components, of course.
;*   after the error-corrected values are obtained, the rgb pointer parameter
;*   is modified to point at 'drgb' so that the lookup and post-lookup-
;*   dithering will use the error-corrected values.
;*
;*   the post-lookup-dithering takes the weighted difference between the
;*   closest color we found and the color we were trying to find, as follows:
;*	 rederr = 3*(drgb.r - foundrgb->r)/4;
;*   this formula is applied to each of the components, and the values are
;*   saved for use in the next call.
;*
;*   the cached lookup reduces the 24-bit rgb value to an 18-bit value used
;*   for the actual lookup (ie, we punt some significance for speed), and a
;*   12-bit hash key to use in the cache data lookup.  the hash key is made
;*   up of the four lowest bits of the 6-bit reduced rgb components.  (ie,
;*   from the original 8-bit components, the hash keys are the bits xxKKKKxx).
;*   the twelve bit hash key then indexes into the table of 4096 cache
;*   domains.  each domain contains two slots, and each slot occupies four
;*   bytes, giving a cache data area of 32k.  within each cache slot, the
;*   four bytes are mapped out such that the first byte is the index of the
;*   closest color found to match the rgb value, and the next 3 bytes hold
;*   the rgb value itself (scaled down to 64-level/6-bit values, remember).
;*   thus, when loaded as a dword, the closest color index ends up in the low
;*   byte of the register and the rgb ends up high-aligned in the upper 18
;*   bits of the register in bgr order.  this doesn't really matter since the
;*   values in the cache data area are visible only to us in this routine,
;*   and we always deal with them packed into this particular sequence. (see
;*   the comments on the barrel-shifter-torture-test code, below).
;*
;*   once the 12 bit key is built, it is multiplied by 8 to index to the
;*   correct domain within the cache data area, and the value from the first
;*   slot in the domain is loaded.  the rgb value in the slot is compared
;*   to the 18-bit rgb we're looking for.  if they match, the closest index
;*   number pulled from that cache slot is returned.  if the rgb value in the
;*   first cache slot is not a match, we check the other slot.	if that rgb
;*   value matches, we return that slot's closest index.  if the second slot's
;*   rgb value doesn't match either, we have a total cache miss.  in this
;*   case, the data from the first cache slot is transferred to the second
;*   slot, and the closestc() routine is used to find the color in the color
;*   table.  the index returned by closestc() is stored into the first cache
;*   slot.  this shuffling of entries on a cache miss implements a very simple
;*   (and cheap in terms of cycles) LRU flushing mechanism.
;*
;*   there is no special 'valid' field within the cache to mark slots that
;*   are used; the cache data is sort of self-validating, for lack of a
;*   better term.  the cache starts out initialized to zeros.  when we
;*   compare a requested color to the value in a slot that has never been
;*   used, the values would match only if the color being searched for is
;*   also all zeros.  this, of course, can only happen for domain zero; in any
;*   other slot that has been used, the rgb values cannot be all zeros or they
;*   wouldn't have hashed to that slot.  domain zero is therefore handled as a
;*   special case.  (if it weren't, a lookup for rgb {0,0,0} could end up
;*   returning index 0 from the cache, and there's no guarantee that the
;*   first palette slot holds {0,0,0} really).	the special case handling is
;*   simple:  the rgb values occupy the upper 18 bits of the dword, and the
;*   index occupies the lower 8 bits, leaving 6 bits inbetween that will
;*   always be zero because of the way we pack 24 bits down to 18.  those bits
;*   are included in the compare of the requested value against the value in
;*   the cache data, but since it is the requested values that eventually
;*   end up getting stored as we makes entries in the cache, those bits of
;*   each dword always remain zeros.  so, when we init the cache (in bhash.c)
;*   we just set the first two cache slots to values that have one of those
;*   bits on, which means that the first time a request is made to find color
;*   {0,0,0}, the compare against what's in the cache slots will be unequal,
;*   forcing a lookup.	once the lookup has filled in a real value, {0,0,0}
;*   lookups will work fine after that.
;*
;*   well, now the comments are longer than the code, so on with it...
;*
;*   oh yeah -- the order of things in the code below is tuned towards the
;*   idea of fastest performance when dithering is turned off and with the
;*   assumption that the first cache slot will hold a hit most of the time.
;*   it looks a bit disordered, but the idea is to take the fall-thru case
;*   of most branches for the conditions we expect to happen the most.
;*
;*****************************************************************************

	align 4
bclosest_col proc near

	Entry
	BArgs	#rgb,#count,#dither
	Save	ebx,edi

;	inc	bhashctl.calls		; when gathering stats, count call

	test	bptr #dither,0FFh	; is dithering turned on?
	jnz	#dodither1		; yes, go do it.

;-----------------------------------------------------------------------------
; lookup code part 1: build the 64-level color and 16-level cache key...
;   in the comments for this section, 'x's represent don't-care bits.
;-----------------------------------------------------------------------------

#buildkeys:

	mov	eax,#rgb		; get source rgb value and transform
	xor	ebx,ebx 		; it into 18-bit and 12-bit keys...
	mov	edi,bhashctl.cachedata	; (this used later, coded here for speed)
	xor	ecx,ecx
	mov	eax,[eax]		;eax xxxxxxxx bbbbbbbb gggggggg rrrrrrrr
	shr	eax,2			;eax 00xxxxxx xxbbbbbb bbgggggg ggrrrrrr
	shrd	ebx,eax,6		;ebx rrrrrr00 00000000 00000000 00000000
	shrd	ecx,eax,4		;ecx rrrr0000 00000000 00000000 00000000
	shr	eax,8			;eax 00000000 00xxxxxx xxbbbbbb bbgggggg
	shrd	ebx,eax,6		;ebx ggggggrr rrrr0000 00000000 00000000
	shrd	ecx,eax,4		;ecx ggggrrrr 00000000 00000000 00000000
	shr	eax,8			;eax 00000000 00000000 00xxxxxx xxbbbbbb
	shrd	ebx,eax,6		;ebx bbbbbbgg ggggrrrr rr000000 00000000
	shrd	ecx,eax,4		;ecx bbbbgggg rrrr0000 00000000 00000000
	shr	ecx,20			;ecx 00000000 00000000 0000bbbb ggggrrrr

;-----------------------------------------------------------------------------
; at this point the transistors in the barrel shifter are smoking, but we've
; got our 64-level colors and our cache key built, see if we have a cache hit.
;-----------------------------------------------------------------------------

	xor	eax,eax 		; cleanup high order eax for return val.
	lea	edi,[ecx*8+edi] 	; make pointer to cache domain/slot 1.
	mov	ecx,[edi]		; get the value from slot 1, move the
	mov	al,cl			; index part to eax for return value,
	mov	cl,0			; then clear it from ecx for compare.
	cmp	ebx,ecx 		; compare rgb we're looking for to the
	jne	short #miss1		; one in the slot. if unequal, go check
;	inc	bhashctl.hits1		; slot 2 in this domain, else have hit.
#found:
	test	bptr #dither,0FFh	; is dithering turned on? if so, go
	jnz	short #dodither2	; calc error value for next time.
#done:
	Restore ebx,edi 		; restore registers,
	Exit				; return color index in eax.

#miss1: 				; if slot one holds a miss, come here.

	mov	cl,al			; restore index so we can shuffle slot
	mov	edx,[edi+4]		; 1 into slot 2 later.	get value from
	mov	al,dl			; slot 2, move the index part to eax,
	mov	dl,0			; then clear it from edx for compare.
	cmp	ebx,edx 		; compare slot 2 rgb value to the one
	jne	short #lookup		; we're looking for, if unequal, go do
;	inc	bhashctl.hits2		; lookup in full color table, else
	jmp	short #found		; we have a hit, go return it.

#lookup:				; if both cache slots missed, come here.

;	inc	bhashctl.misses
	mov	[edi+4],ecx		; shuffle 1st slot entry into 2nd slot.
	push	#count			; set up to call the closestc()
	push	bhashctl.ctab		; routine to search the full color
	push	#rgb			; table for the 24-bit version of the
	call	closestc		; value we're looking for.  on return
	add	esp,12			; merge the index of the closest color
	mov	bl,al			; with the 18-bit version of the value
	mov	[edi],ebx		; we're looking for, and store them
	jmp	short #found		; into slot 1 of the cache domain.

;-----------------------------------------------------------------------------
; error diffusion dithering part 2: remember current error for next time...
;  this is done after we've found the color closest to the one we wanted.
;-----------------------------------------------------------------------------

#dodither2:

	push	eax			; save closest-index (return value)

	lea	eax,[eax*2+eax] 	; multiply color index by 3 (quickly).
	lea	edi,bhashctl		; load pointer to our hash control,
	mov	ecx,[edi].ctab		; load pointer to full color pallete.
	mov	ecx,[ecx+eax]		; get rgb value we found as closest.

	mov	ebx,[edi].drgb		; get rgb value we wanted find.

	movzx	eax,bl			; load red value we wanted.
	movzx	edx,cl			; load red value we found.
	sub	eax,edx 		; calc the difference.
	lea	eax,[eax*2+eax] 	; multiply by three.
	sar	eax,2			; divide by four.
	mov	[edi].rederr,eax	; save red error for next call.

	movzx	eax,bh			; load green value we wanted.
	movzx	edx,ch			; load green value we found.
	sub	eax,edx 		; calc the difference.
	lea	eax,[eax*2+eax] 	; multiply by three.
	sar	eax,2			; divide by four.
	mov	[edi].grnerr,eax	; save green error for next call

	shr	ebx,8			; shuffle blue values down into the
	shr	ecx,8			; byte-addressable part of the regs.
	movzx	eax,bh			; load blue value we wanted.
	movzx	edx,ch			; load blue value we found.
	sub	eax,edx 		; calc the difference.
	lea	eax,[eax*2+eax] 	; multiply by three.
	sar	eax,2			; divide by four.
	mov	[edi].bluerr,eax	; save blue error for next call

	pop	eax			; restore closest-index return value.
	jmp	#done			; go return it.

;-----------------------------------------------------------------------------
; error diffusion dithering, part 1: roll in the error from last time...
;-----------------------------------------------------------------------------

#dodither1:

	lea	edi,bhashctl		; load pointer to cache control.
	mov	ecx,#rgb		; load pointer to requested color.
	mov	ecx,[ecx]		; load rgb values of requested color.
	mov	edx,255 		; we use 255 a lot, registerize it.

	movzx	eax,cl			; load requested red value, add
	add	eax,[edi].rederr	; error from last time, if result is
	js	short #redneg		; negative, go force to 0, else if
	cmp	eax,edx 		; result is 255 or less, we're good,
	jle	short #redgood		; else we force the value to 255. the
#redneg:				; forcing of 0 or 255 is a bit tricky:
	sets	al			; the SETS will set AL to 1 or 0,
	dec	al			; the DEC makes it 0 or 255 (0FFh).
#redgood:
	mov	cl,al			; save new red value to search for.

	movzx	eax,ch			; load requested green value, add
	add	eax,[edi].grnerr	; error from last time, if result is
	js	short #grnneg		; negative, go force to 0, else if
	cmp	eax,edx 		; result is 255 or less, we're good,
	jle	short #grngood		; else we force the value to 255. the
#grnneg:				; forcing of 0 or 255 is a bit tricky,
	sets	al			; the SETS will set al to 1 or 0,
	dec	al			; the DEC makes it 0 or 255 (0FFh).
#grngood:
	mov	ch,al			; save new green value to search for.

	ror	ecx,8			; roll blue into byte-addressable range.
	movzx	eax,ch			; load requested blue value, add
	add	eax,[edi].bluerr	; error from last time, if result is
	js	short #bluneg		; negative, go force to 0, else if
	cmp	eax,edx 		; result is 255 or less, we're good,
	jle	short #blugood		; else we force the value to 255. the
#bluneg:				; forcing of 0 or 255 is a bit tricky,
	sets	al			; the SETS will set al to 1 or 0,
	dec	al			; the DEC makes it 0 or 255 (0FFh).
#blugood:
	mov	ch,al			; save new blue value to search for.
	rol	ecx,8			; roll blue back up where it belongs.

	lea	eax,[edi].drgb		; load pointer to our dithered-rgb
	mov	[eax],ecx		; save area, save the corrected values.
	mov	#rgb,eax		; adjust the requested-color pointer
	jmp	#buildkeys		; to point at our dithered values.

bclosest_col endp

_TEXT	ends
	end

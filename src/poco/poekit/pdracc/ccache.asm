;*****************************************************************************
;* CCACHE.ASM - Cached color fitting, dithering, and histogramming code.
;*****************************************************************************

POE_USAGE = 1	; 0 for use on the host side, 1 for use in POE modules.

	include stdmacro.i

;-----------------------------------------------------------------------------
; data and constants...
;
;   Notes:
;     It is critical that C64HISTSIZE==C64CACHESIZE and likewise for the
;     256-level sizes.	They are given separate names mainly as documentation
;     for how the numbers are derived.	But, since the histogram and cache
;     buffer are the same memory space (at different times), they must match.
;-----------------------------------------------------------------------------

_DATA	segment

C64HISTSIZE   = 64*64*64		; 64 levels, 1 byte per entry
C256HISTSIZE  = 256*256*256/8		; 256 levels, 1 bit per entry
C64CACHESIZE  = 64*64*64*1*1		; 64-level keys, 1 byte per cache slot
C256CACHESIZE = 64*64*64*2*4		; 64-level keys, 2 dwords per cache slot

cctl	struc				; the cache control structure...
ctab	  dd	?			; -> color table to cache
ccount	  dd	?			; number of colors in table
cfitline  dd	?			; -> routine to fit rgb line to ctab
histline  dd	?			; -> routine to update histogram
cfitinit  dd	?			; -> routine to init color fitting
histinit  dd	?			; -> routine to init histogram
histccnt  dd	?			; -> routine to return hist color count
hst2ctab  dd	?			; -> routine to convert hist to ctab
data	  dd	?			; -> histogram/cachedata
datasize  dd	?			; size of histogram/cachedata
rederr	  dd	?			; for dithering, last red error
grnerr	  dd	?			; last green error
bluerr	  dd	?			; last blue error
drgb	  dd	?			; dithered rgb value (work var)
;calls	   dd	 ?			; these used for cache stats...
;misses    dd	 ?
;totcolors dd	 ?
cctl	ends

cc	  cctl	<>			; cache ctrl struct, statically alloc'd.

bittab	  db	80h,40h,20h,10h,08h,04h,02h,01h ; used by 256-level hist stuff.

_DATA	ends

;-----------------------------------------------------------------------------
; code...
;
;   Notes:
;     Because this code is used both by the host, and by Ian in POE modules,
;     a couple macros are defined below for memory alloc/free, so that
;     one need not dig through the code and change things.  The macros
;     key off the definition of POE_USAGE, defined near the top of the module.
;-----------------------------------------------------------------------------

_TEXT	segment
	assume cs:CGROUP,ds:DGROUP

	extrn	closestc:near

	public	cc_init
	public	cc_cleanup
	public	cc_cfitinit
	public	cc_histinit
	public	cc_cfitline
	public	cc_histline
	public	cc_hist_color_count
	public	cc_hist_to_ctab

;-----------------------------------------------------------------------------
; some macros for memory works, because the routines to call have different
; names in the host versus poe environments.
;-----------------------------------------------------------------------------

 if POE_USAGE	; if being used in a POE module...

	extrn	malloc:near
	extrn	free:near

MALLOC	macro
	call	malloc
	endm

FREE	macro	ptr
	call	free
	endm

 else		; if being used on the host side...

	extrn	pj_malloc:near
	extrn	pj_free:near

MALLOC	macro
	call	pj_malloc
	endm
FREE	macro	ptr
	call	pj_free
	endm

 endif		; end of poe/host compatibility

;*****************************************************************************
;* void c64hinit(void) - init histogram
;*****************************************************************************

c64hinit proc near

	Entry
	Save	edi

	xor	eax,eax
	mov	ecx,C64HISTSIZE/4
	mov	edi,cc.data
	rep stosd

	Restore edi
	Exit

c64hinit endp

;*****************************************************************************
;* void c256hinit(void) - init histogram
;*****************************************************************************

c256hinit proc near

	Entry
	Save	edi

	xor	eax,eax
	mov	ecx,C256HISTSIZE/4
	mov	edi,cc.data
	rep stosd

	Restore edi
	Exit

c256hinit endp

;*****************************************************************************
;* void c64cinit(void) - init color fitting cache
;*****************************************************************************

c64cinit proc near

	Entry
	Save	edi

	xor	eax,eax
	not	eax			; this cache gets init'd to 0xFF...
	mov	ecx,C64HISTSIZE/4
	mov	edi,cc.data
	rep stosd

	Restore edi
	Exit

c64cinit endp

;*****************************************************************************
;* void c256cinit(void) - init color fitting cache
;*****************************************************************************

c256cinit proc near

	Entry
	Save	edi

	xor	eax,eax
	mov	ecx,C256HISTSIZE/4
	mov	edi,cc.data
	rep stosd

	Restore edi
	Exit

c256cinit endp

;*****************************************************************************
;* void c64histline(Rgb3 *lbuf, int width)
;*   update histogram with rgb colors found in the line buffer.
;*****************************************************************************

c64histline proc near

	Entry
	Args	#lbuf,#width
	Save	esi,edi

	mov	ecx,#width
	mov	esi,#lbuf
	mov	edi,cc.data
	align 4
#loop:
	dec	ecx
	js	short #done
	xor	edx,edx
	mov	eax,[esi]
	add	esi,3
	shl	eax,8
	jz	short #setit		; early out for black pixels
	shld	edx,eax,6
	shl	eax,8
	shld	edx,eax,6
	shl	eax,8
	shld	edx,eax,6
#setit:
	mov	bptr [edi+edx],0FFh
	jmp	short #loop
#done:
	Restore esi,edi
	Exit

c64histline endp

;*****************************************************************************
;* void c256histline(Rgb3 *lbuf, int width)
;*   update histogram with rgb colors found in the line buffer.
;*****************************************************************************

c256histline proc near

	Entry
	Args	#lbuf,#width
	Save	esi,edi

	mov	ecx,#width
	mov	esi,#lbuf
	mov	edi,cc.data
	xor	edx,edx
	align 4
#loop:
	dec	ecx
	js	short #done
	mov	eax,[esi]
	add	esi,3
	mov	dl,al
	and	dl,7
	and	eax,00FFFFFFh
	shr	eax,3
	mov	dl,[edx+bittab]
	or	bptr [edi+eax],dl
	jmp	short #loop
#done:
	Restore esi,edi
	Exit

c256histline endp

;*****************************************************************************
;* int hist64_entries(void) - return count of colors recorded in histogram
;*****************************************************************************

hist64_entries proc near

	Entry
	Save	esi

	mov	esi,cc.data
	mov	ecx,C64HISTSIZE/4
	xor	edx,edx
	align 4
#loop:
	lodsd
	shl	eax,1
	adc	edx,0
	shl	eax,8
	adc	edx,0
	shl	eax,8
	adc	edx,0
	shl	eax,8
	adc	edx,0
	dec	ecx
	jnz	short #loop

	mov	eax,edx

	Restore esi
	Exit

hist64_entries endp

;*****************************************************************************
;* int hist256_entries(void) - return count of colors recorded in histogram
;*****************************************************************************

	align 4
hist256_entries proc near

	Entry
	Save	ebx,esi,edi

	xor	edx,edx
	mov	ebx,C256HISTSIZE/4
	xor	ecx,ecx
	xor	edi,edi
	mov	esi,cc.data
	align 4
#mainloop:
	dec	ebx
	js	short #done
	lodsd
	test	eax,eax
	jz	short #mainloop
	mov	cl,4
	align 4
#bitsloop:
	shr	eax,1
	adc	edx,edi
	shr	eax,1
	adc	edx,edi
	shr	eax,1
	adc	edx,edi
	shr	eax,1
	adc	edx,edi
	shr	eax,1
	adc	edx,edi
	shr	eax,1
	adc	edx,edi
	shr	eax,1
	adc	edx,edi
	shr	eax,1
	adc	edx,edi
	dec	ecx
	jnz	short #bitsloop
	jmp	short #mainloop
#done:
	mov	eax,edx
	Restore ebx,esi,edi
	Exit

hist256_entries endp


;*****************************************************************************
;* void hist64_to_ctab(void) - build ctab containing all colors in histogram
;*****************************************************************************

hist64_to_ctab proc near

	Entry
	Args	#big_ctab
	Save	ebx,esi,edi

	mov	esi,cc.data
	mov	edi,#big_ctab

	xor	ebx,ebx 		; blue
	xor	edx,edx 		; green and red
	align 4
#loop:
	lodsb
	test	al,al
	jz	short #skip
	mov	[edi],dl		; red
	mov	[edi+1],dh		; green
	mov	[edi+2],bl		; blue
	add	edi,3
#skip:
	add	dl,4			; incr red
	jnz	short #loop
	add	dh,4			; incr green
	jnz	short #loop
	add	bl,4			; incr blue
	jnz	short #loop

	Restore ebx,esi,edi
	Exit

hist64_to_ctab endp

;*****************************************************************************
;* void hist256_to_ctab(void) - build ctab containing all colors in histogram
;*****************************************************************************

hist256_to_ctab proc near

	Entry
	Args	#big_ctab
	Save	ebx,esi,edi

	mov	esi,cc.data
	mov	edi,#big_ctab

	xor	ebx,ebx 		; blue
	xor	edx,edx 		; green and red

#loop:
	lodsb
	test	al,al
	jz	short #nextbyte
	mov	bh,dl
#bits:
	shl	al,1
	jc	short #output
	jz	short #nextbyte
	inc	bh			; incr red bit count
	jmp	short #bits
#output:
	mov	[edi],bh		; red
	mov	[edi+1],dh		; green
	mov	[edi+2],bl		; blue
	add	edi,3
	inc	bh			; incr red bit count
	jmp	short #bits
#nextbyte:
	add	dl,8			; incr red
	jnz	short #loop
	inc	dh			; incr green
	jnz	short #loop
	inc	bl			; incr blue
	jnz	short #loop

	Restore ebx,esi,edi
	Exit

hist256_to_ctab endp

;*****************************************************************************
; _dither1 - internal service routine used by dither-fit routines.
;
;   error diffusion dithering, part 1: rolls in the error from last time...
;
; Entry:
;   esi - pointer to input rgb data
; Exit:
;   eax - adjusted rgb values (also saved in cc.drgb)
;   ebx - pointer to cc (<---NOTE EBX MODIFIED!!!)
;   ecx - trashed
;   edx - trashed
;   others unmodified
;*****************************************************************************

	align 4
_dither1 proc near

	lea	ebx,cc			; load pointer to cache control.
	mov	eax,[esi]		; load rgb values of requested color.
	mov	edx,255 		; we use this a lot, registerize it.

	movzx	ecx,al			; load requested red value, add
	add	ecx,[ebx].rederr	; error from last time, if result is
	js	short #redneg		; negative, go force to 0, else if
	cmp	ecx,edx 		; result is 255 or less, we're good,
	jle	short #redgood		; else we force the value to 255. the
#redneg:				; forcing of 0 or 255 is a bit tricky:
	sets	cl			; the SETS will set AL to 1 or 0,
	dec	cl			; the DEC makes it 0 or 255 (0FFh).
#redgood:
	mov	al,cl			; save new red value to search for.

	movzx	ecx,ah			; load requested green value, add
	add	ecx,[ebx].grnerr	; error from last time, if result is
	js	short #grnneg		; negative, go force to 0, else if
	cmp	ecx,edx 		; result is 255 or less, we're good,
	jle	short #grngood		; else we force the value to 255. the
#grnneg:				; forcing of 0 or 255 is a bit tricky,
	sets	cl			; the SETS will set al to 1 or 0,
	dec	cl			; the DEC makes it 0 or 255 (0FFh).
#grngood:
	mov	ah,cl			; save new green value to search for.

	ror	eax,8			; roll blue into byte-addressable range.
	movzx	ecx,ah			; load requested blue value, add
	add	ecx,[ebx].bluerr	; error from last time, if result is
	js	short #bluneg		; negative, go force to 0, else if
	cmp	ecx,edx 		; result is 255 or less, we're good,
	jle	short #blugood		; else we force the value to 255. the
#bluneg:				; forcing of 0 or 255 is a bit tricky,
	sets	cl			; the SETS will set al to 1 or 0,
	dec	cl			; the DEC makes it 0 or 255 (0FFh).
#blugood:
	mov	ah,cl			; save new blue value to search for.
	rol	eax,8			; roll blue back up where it belongs.

	mov	[ebx].drgb,eax		; save the corrected values.
	ret

_dither1 endp

;*****************************************************************************
; _dither2 - internal service routine used by dither-fit routines.
;   error diffusion dithering part 2: remember current error for next time...
;
; Entry:
;   eax - index of closest color we could find in the palette
; Exit:
;   cc.rederr,grnerr,bluerr hold error values for next time.
;   eax,ebx,ecx,edx trashed. (<--NOTE EBX TRASHED!!!)
;   others unmodified
;*****************************************************************************

_dither2 proc near

	lea	eax,[eax*2+eax] 	; multiply color index by 3 (quickly).
	mov	ecx,cc.ctab		; load pointer to full color pallete.
	mov	ecx,[ecx+eax]		; get rgb value we found as closest.

	mov	ebx,cc.drgb		; get rgb value we wanted find.

	movzx	eax,bl			; load red value we wanted.
	movzx	edx,cl			; load red value we found.
	sub	eax,edx 		; calc the difference.
	lea	eax,[eax*2+eax] 	; multiply by three.
	sar	eax,2			; divide by four.
	mov	cc.rederr,eax		; save red error for next call.

	movzx	eax,bh			; load green value we wanted.
	movzx	edx,ch			; load green value we found.
	sub	eax,edx 		; calc the difference.
	lea	eax,[eax*2+eax] 	; multiply by three.
	sar	eax,2			; divide by four.
	mov	cc.grnerr,eax		; save green error for next call

	shr	ebx,8			; shuffle blue values down into the
	shr	ecx,8			; byte-addressable part of the regs.
	movzx	eax,bh			; load blue value we wanted.
	movzx	edx,ch			; load blue value we found.
	sub	eax,edx 		; calc the difference.
	lea	eax,[eax*2+eax] 	; multiply by three.
	sar	eax,2			; divide by four.
	mov	cc.bluerr,eax		; save blue error for next call

	ret

_dither2 endp

;*****************************************************************************
;* void c64ditherline(Pixel *pbuf, Rgb3 *lbuf, int width)
;*   translate the rgb values in lbuf into pixels in pbuf by fitting to the
;*   cache color table, applying error correction dithering in the process.
;*****************************************************************************

c64ditherline proc near

	Entry
	Args	#pbuf,#lbuf,#width
	Save	ebx,esi,edi

	lea	edi,cc.rederr		; flush dither error from prior line...
	xor	eax,eax
	stosd
	stosd
	stosd

;	mov	eax,#width		; we'll do a whole line's worth of
;	add	cc.calls,eax		; cached lookup calls, count them.

	mov	edi,#pbuf
	mov	esi,#lbuf

#mainloop:

	call	_dither1		; get next rgb, roll in error dither
	add	esi,3			; increment input pointer.

	xor	ecx,ecx
	shl	eax,8			; reduce 24-bit rgb to 18-bit values...
	jz	short #havekey		; early out for black pixels
	shld	ecx,eax,6
	shl	eax,8
	shld	ecx,eax,6
	shl	eax,8
	shld	ecx,eax,6
#havekey:
	mov	edx,[ebx].data		; load pointer to cache data
	movzx	eax,bptr [ecx+edx]	; get cached closest color index
	cmp	al,0FFh 		; if anything but 0xFF, slot has been
	jne	short #hit		; used, and is thus a valid hit.
#lookup:
;	inc	[ebx].misses
	add	edx,ecx
	push	edx			; save pointer to cache slot

	lea	eax,[ebx].drgb
	push	[ebx].ccount
	push	[ebx].ctab
	push	eax
	call	closestc		; look up closest color in palette
	add	esp,12
	pop	edx
	mov	[edx],al
#hit:
	stosb				; save closest color index
	call	_dither2		; calc error for next dither
	dec	dptr #width		; decrement line width counter,
	jnz	short #mainloop 	; loop till line is done.

	Restore ebx,esi,edi
	Exit

c64ditherline endp

;*****************************************************************************
;* void c64fitline(Pixel *pbuf, Rgb3 *lbuf, int width)
;*   translate the rgb values in lbuf into pixels in pbuf by fitting to the
;*   cache color table.
;*****************************************************************************

c64cfitline proc near

	Entry
	Args	#pbuf,#lbuf,#width
	Save	ebx,esi,edi,ebp

	mov	esi,#lbuf
	mov	edi,#pbuf
	mov	ebp,#width
	lea	ebx,cc
;	add	[ebx].calls,ebp
	mov	edx,[ebx].data
	jmp	short #loop

#done:
	Restore ebx,esi,edi,ebp
	Exit

	align 4
#hit:
	stosb
	add	esi,3
#loop:
	dec	ebp
	js	short #done
	xor	ecx,ecx
	mov	eax,[esi]
	shl	eax,8
	jz	short #havekey		; early out for black pixels
	shld	ecx,eax,6
	shl	eax,8
	shld	ecx,eax,6
	shl	eax,8
	shld	ecx,eax,6
#havekey:
	mov	al,[ecx+edx]
	cmp	al,0FFh
	jne	short #hit
#lookup:
;	inc	[ebx].misses
	push	ecx
	push	edx

	push	[ebx].ccount
	push	[ebx].ctab
	push	esi
	call	closestc
	add	esp,12

	pop	edx
	pop	ecx
	mov	[ecx+edx],al
	jmp	short #hit

c64cfitline endp

;*****************************************************************************
;* void c256ditherline(Pixel *pbuf, Rgb3 *lbuf, int width)
;*   translate the rgb values in lbuf into pixels in pbuf by fitting to the
;*   cache color table, applying error correction dithering in the process.
;*****************************************************************************

c256ditherline proc near

	Entry
	Args	#pbuf,#lbuf,#width
	Save	ebx,esi,edi

;	mov	eax,#width
;	add	cc.calls,eax

	mov	esi,#lbuf
	mov	edi,#pbuf
#loop:
	call	_dither1		; get next rgb, roll in error dither
	add	esi,3			; increment input pointer.

	xor	edx,edx
	shl	eax,8
	mov	ebx,eax 		; save upshifted color for compare
	jz	short #havekey		; early out for black pixels
	shl	eax,2
	shld	edx,eax,6
	shl	eax,8
	shld	edx,eax,6
	shl	eax,8
	shld	edx,eax,6
	shl	edx,3			; multiply by 8; cache entries are
#havekey:
	add	edx,cc.data		; two dwords each.

	mov	ecx,[edx+4]		; get 2nd cache entry
	movzx	eax,cl
	mov	cl,0
	cmp	ecx,ebx
	je	short #hit
	mov	ecx,[edx]		; get 1st cache entry
	movzx	eax,cl
	mov	cl,0
	cmp	ecx,ebx
	je	short #hit
	mov	[edx+4],eax		; total miss: shuffle 1st entry to 2nd.

;	inc	cc.misses
	push	edx

	lea	ecx,cc
	lea	eax,[ecx].drgb
	push	[ecx].ccount
	push	[ecx].ctab
	push	eax
	call	closestc
	add	esp,12

	pop	edx
	mov	bl,al
	mov	[edx],ebx
#hit:
	stosb
	call	_dither2
	dec	dptr #width
	jnz	short #loop
#done:
	Restore ebx,esi,edi
	Exit

	ret
c256ditherline endp

;*****************************************************************************
;* void c256fitline(Pixel *pbuf, Rgb3 *lbuf, int width)
;*   translate the rgb values in lbuf into pixels in pbuf by fitting to the
;*   cache color table.
;*****************************************************************************

c256cfitline proc near

	Entry
	Args	#pbuf,#lbuf,#width
	Save	ebx,esi,edi

;	mov	eax,#width
;	add	cc.calls,eax

	mov	esi,#lbuf
	mov	edi,#pbuf
#loop:
	mov	eax,[esi]
	xor	edx,edx
	shl	eax,8
	mov	ebx,eax 		    ; save upshifted color for compare
	jz	short #havekey		    ; early out for black pixels
	shl	eax,2
	shld	edx,eax,6
	shl	eax,8
	shld	edx,eax,6
	shl	eax,8
	shld	edx,eax,6
	shl	edx,3			    ; multiply by 8; cache entries are
#havekey:
	add	edx,cc.data		    ; two dwords each.

	mov	ecx,[edx+4]		    ; get 2nd cache entry
	mov	eax,ecx
	mov	cl,0
	cmp	ecx,ebx
	je	short #hit
	mov	ecx,[edx]		    ; get 1st cache entry
	mov	eax,ecx
	mov	cl,0
	cmp	ecx,ebx
	je	short #hit
	mov	[edx+4],eax		    ; total miss: shuffle 1st entry to 2nd.

;	inc	cc.misses
	push	edx

	push	cc.ccount
	push	cc.ctab
	push	esi
	call	closestc
	add	esp,12

	pop	edx
	mov	bl,al
	mov	[edx],ebx
#hit:
	stosb
	add	esi,3
	dec	dptr #width
	jnz	short #loop
#done:
	Restore ebx,esi,edi
	Exit

c256cfitline endp

;*****************************************************************************
;* jump_table - this 'proc' contains a bunch of jump statements that vector
;*		our publicly-visible routines to the actual routines set up
;*		at init time.  the label on each of these jump statements
;*		is declared as public, above.  beside the init and cleanup
;*		routines, these labels represent the only face we show to
;*		the outside world.
;*****************************************************************************

jump_table proc near

cc_cfitinit:	      jmp     dptr cc.cfitinit
cc_histinit:	      jmp     dptr cc.histinit
cc_cfitline:	      jmp     dptr cc.cfitline
cc_histline:	      jmp     dptr cc.histline
cc_hist_color_count:  jmp     dptr cc.histccnt
cc_hist_to_ctab:      jmp     dptr cc.hst2ctab

jump_table endp

;*****************************************************************************
;* void cc_cleanup(void) - free the histogram/cache data area (if any)
;*****************************************************************************

cc_cleanup proc near

	Entry
	mov	eax,cc.data
	test	eax,eax
	jz	short #done
	push	eax
	FREE
	add	esp,4
	mov	cc.data,0
#done:
	Exit

cc_cleanup endp

;*****************************************************************************
;* Errcode cc_init(Cmap *cmap, Boolean colors256, Boolean dodither)
;*   fill in the fields of the CacheControl structure as appropriate for
;*   the requested colors256 and dodither options, and allocate the
;*   histogram/cachedata buffer at the appropriate size.
;*****************************************************************************

Setvect macro	vector,address
	lea	eax,address
	mov	[edx].vector,eax
	endm
Setdata macro	field,value
	mov	[edx].field,value
	endm

cc_init proc near

	Entry
	Args	#cmap,#colors256,#dodither

	lea	edx,cc

;	xor	eax,eax
;	Setdata calls,eax
;	Setdata misses,eax
;	Setdata totcolors,eax

	mov	ecx,#cmap
	lea	eax,[ecx+4]
	mov	ecx,[ecx]
	Setdata ctab,eax
	Setdata ccount,ecx

	mov	eax,#colors256
	mov	ecx,#dodither
	test	eax,eax
	jnz	short #setup256

#setup64:
	lea	eax,c64cfitline
	test	ecx,ecx
	jz	short #nodither64
	lea	eax,c64ditherline
#nodither64:
	Setdata cfitline,eax
	Setvect histline,c64histline
	Setvect cfitinit,c64cinit
	Setvect histinit,c64hinit
	Setvect histccnt,hist64_entries
	Setvect hst2ctab,hist64_to_ctab
	mov	eax,C64HISTSIZE
	Setdata datasize,eax
	jmp	short #getmemory

#setup256:
	lea	eax,c256cfitline
	test	ecx,ecx
	jz	short #nodither256
	lea	eax,c256ditherline
#nodither256:
	Setdata cfitline,eax
	Setvect histline,c256histline
	Setvect cfitinit,c256cinit
	Setvect histinit,c256hinit
	Setvect histccnt,hist256_entries
	Setvect hst2ctab,hist256_to_ctab
	mov	eax,C256HISTSIZE
	Setdata datasize,eax

#getmemory:
	push	eax
	MALLOC
	add	esp,4
	mov	cc.data,eax
	test	eax,eax
	jz	short #allocerr
	xor	eax,eax
	jmp	short #done
#allocerr:
	mov	eax,-2			; err_no_memory
#done:
	Exit

cc_init endp

_TEXT	ends

	end

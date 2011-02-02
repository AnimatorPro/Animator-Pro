;*****************************************************************************
;* DRVBLTA.ASM - Code having to do with the higher-order blit functions.
;*
;*  NOTES:
;*		This code contains several driver routines, and several
;*		service routines which assist the blit code in DRVBLTC.C.
;*
;*		In general, the common driver code which supports the higher-
;*		order blits is not tuned to the same degree as the low-level
;*		driver routines.  The idea here is to outperform Animator's
;*		generic library routines, and this is generally accomplished
;*		by working on larger chunks of data at a time (the generics
;*		typically work on 1 line at a time using a 1k blit buffer).
;*		We use a 16k blit buffer and do a group of lines at a time,
;*		with the primary goal of doing less bank switches, especially
;*		on cards that support a single window for reads and writes.
;*
;*		Several routines within the common higher-order blits use a
;*		deltabpr.  This is used to avoid a multiply instruction
;*		within a loop to get to the next line in a bytemap raster.
;*		The idea is that after processing a segment of a line in a
;*		bytemap, the pointer is left pointing to the next byte after
;*		the segment.  We need to get it to the start of the same
;*		segment on the next line, so the deltabpr is the number we
;*		add to get there.  (EG, if the bytemap is 320x200, and a blit
;*		is being done with an X coordinate of 10 and a width of 300,
;*		the deltabpr is 20.  When we get done with a segment, the
;*		pointer will point to pixel 310 on the current line, and
;*		adding 20 will get to pixel 10 on the next line.)
;*
;*  MAINTENANCE:
;*    03/27/91	Ian Lepore
;*		Basically a total re-write.
;*****************************************************************************

;******************************************************************************
;*									      *
;*		   Copyright (C) 1991 by Autodesk, Inc. 		      *
;*									      *
;*	Permission to use, copy, modify, and distribute this software and     *
;*	its documentation for the purpose of creating applications for	      *
;*	Autodesk Animator, is hereby granted in accordance with the terms     *
;*	of the License Agreement accompanying this product.		      *
;*									      *
;*	Autodesk makes no warrantees, express or implied, as to the	      *
;*	correctness of this code or any derivative works which incorporate    *
;*	it.  Autodesk provides the code on an ''as-is'' basis and             *
;*	explicitly disclaims any liability, express or implied, for	      *
;*	errors, omissions, and other problems in the code, including	      *
;*	consequential and incidental damages.				      *
;*									      *
;*	Use, duplication, or disclosure by the U.S.  Government is	      *
;*	subject to restrictions set forth in FAR 52.227-19 (Commercial	      *
;*	Computer Software - Restricted Rights) and DFAR 252.227-7013 (c)      *
;*	(1) (ii) (Rights in Technical Data and Computer Software, as	      *
;*	applicable.							      *
;*									      *
;******************************************************************************

	include stdmacro.i
	include drvcomn.i
	include raster.i

_TEXT	segment

	public	pj_vdrv_zoom_line
	public	pj_vdrv_tblit_line
	public	pj_vdrv_put_rectram
	public	pj_vdrv_blit_to_ram
	public	pj_vdrv_blit_from_ram
	public	pj_vdrv_mask1line
	public	pj_vdrv_mask2line
	public	pj_vdrv_memcpy

;****************************************************************************
;* void pj_vdrv_mask1line(void);
;*  Expand a bitmask into a line of pixels on the screen, where 1's in the
;*  bitmask result in 'oncolor' and 0's in the bitmask leave the corresponding
;*  pixel unchanged.
;****************************************************************************

pj_vdrv_mask1line proc near

	Entry
	Save	ebx,esi,edi

	lea	edx,pj_vdrv_wcontrol
	mov	edi,[edx].ry
	mov	eax,[edi*8+ytab_bank]
	mov	edi,[edi*8+ytab_offs]
	add	edi,[edx].rx
	test	eax,0FFFF0000h
	jnz	#slowblit
	cmp	eax,[edx].wwrcurbank
	je	short #fastbankok
	SetWriteBank
#fastbankok:
	add	edi,[edx].wwraddr
	mov	esi,[edx].mbytes	; load pointer to bitmask array
	mov	ecx,[edx].count 	; load count of bits to process
	mov	bl,[edx].bit1		; load starting bit-test mask value
	mov	bh,[edx].oncolor	; load color of 'on' pixels
	lodsb				; get first byte from bitmask array
#fastloop:
	test	al,bl			; test current bit in bitmask.
	jz	short #fastnset 	; if zero, leave buffer unchanged,
	mov	gs:[edi],bh		; else put oncolor value into buffer
#fastnset:
	inc	edi			; increment pixel buffer pointer.
	shr	bl,1			; shift to next test bit. if shift
	loopnz	#fastloop		; result & counter both non-zero loop,
	jecxz	short mask1exit 	; else if counter is zero, done, else
	mov	bl,80h			; load a fresh bit-test mask, and get
	lodsb				; the next byte from the mask array,
	jmp	short #fastloop 	; then loop to do some more.
mask1exit:
	Restore ebx,esi,edi
	Exit

#slowblit:
	push	ebp
	mov	ebp,[edx].wwraddr
	and	eax,0000FFFFh
	cmp	eax,[edx].wwrcurbank
	je	short #slowbankok1
	SetWriteBank
#slowbankok1:
	mov	esi,[edx].mbytes	; load pointer to bitmask array
	mov	ecx,[edx].count 	; load count of bits to process
	mov	bl,[edx].bit1		; load starting bit-test mask value
	mov	bh,[edx].oncolor	; load color of 'on' pixels
	lodsb				; get first byte from bitmask array
#slowloop:
	cmp	edi,[edx].woffsmask
	jbe	#slowbankok2
	and	edi,[edx].woffsmask
	push	eax
	mov	eax,[edx].wwrcurbank
	inc	eax
	SetWriteBank
	pop	eax
#slowbankok2:
	test	al,bl			; test current bit in bitmask.
	jz	short #slownset 	; if zero, leave buffer unchanged,
	mov	gs:[edi+ebp],bh 	; else put oncolor value into buffer
#slownset:
	inc	edi			; increment pixel buffer pointer.
	shr	bl,1			; shift to next test bit. if shift
	loopnz	#slowloop		; result & counter both non-zero loop,
	jecxz	short #slowdone 	; else if counter is zero, done, else
	mov	bl,80h			; load a fresh bit-test mask, and get
	lodsb				; the next byte from the mask array,
	jmp	short #slowloop 	; then loop to do some more.
#slowdone:
	pop	ebp
	jmp	short mask1exit

pj_vdrv_mask1line endp


;****************************************************************************
;* void pj_vdrv_mask2line(void);
;*  Expand a bitmask into a line of pixels on the screen, where 1's in the
;*  bitmask result in 'oncolor' and 0's in the bitmask result in offcolor.
;****************************************************************************

pj_vdrv_mask2line proc near

	Entry
	Save	ebx,esi,edi

	lea	edx,pj_vdrv_wcontrol
	mov	edi,[edx].ry
	mov	eax,[edi*8+ytab_bank]
	mov	edi,[edi*8+ytab_offs]
	add	edi,[edx].rx
	test	eax,0FFFF0000h
	jnz	#slowblit
	cmp	eax,[edx].wwrcurbank
	je	short #fastbankok
	SetWriteBank
#fastbankok:
	add	edi,[edx].wwraddr
	mov	esi,[edx].mbytes	; load pointer to bitmask array
	mov	ecx,[edx].count 	; load count of bits to process
	mov	bl,[edx].bit1		; load starting bit-test mask value
	mov	bh,[edx].oncolor	; load color of 'on' pixels
	mov	ah,[edx].offcolor	; load color of 'off' pixels
	lodsb				; get first byte from bitmask array
#fastloop:
	test	al,bl			; test current bit in bitmask.
	jz	short #fastoff		; if zero, leave buffer unchanged,
	mov	gs:[edi],bh		; else put oncolor value into buffer
	jmp	short #fastcontinue
#fastoff:
	mov	gs:[edi],ah
#fastcontinue:
	inc	edi			; increment pixel buffer pointer.
	shr	bl,1			; shift to next test bit. if shift
	loopnz	#fastloop		; result & counter both non-zero loop,
	jecxz	short mask2exit 	; else if counter is zero, done, else
	mov	bl,80h			; load a fresh bit-test mask, and get
	lodsb				; the next byte from the mask array,
	jmp	short #fastloop 	; then loop to do some more.
mask2exit:
	Restore ebx,esi,edi
	Exit

#slowblit:
	push	ebp
	mov	ebp,[edx].wwraddr
	and	eax,0000FFFFh
	cmp	eax,[edx].wwrcurbank
	je	short #slowbankok1
	SetWriteBank
#slowbankok1:
	mov	esi,[edx].mbytes	; load pointer to bitmask array
	mov	ecx,[edx].count 	; load count of bits to process
	mov	bl,[edx].bit1		; load starting bit-test mask value
	mov	bh,[edx].oncolor	; load color of 'on' pixels
	mov	ah,[edx].offcolor	; load color of 'off' pixels
	lodsb				; get first byte from bitmask array
#slowloop:
	cmp	edi,[edx].woffsmask
	jbe	#slowbankok2
	and	edi,[edx].woffsmask
	push	eax
	mov	eax,[edx].wwrcurbank
	inc	eax
	SetWriteBank
	pop	eax
#slowbankok2:
	test	al,bl			; test current bit in bitmask.
	jz	short #slowoff		; if zero, leave buffer unchanged,
	mov	gs:[edi+ebp],bh 	; else put oncolor value into buffer
	jmp	short #slowcontinue
#slowoff:
	mov	gs:[edi+ebp],ah
#slowcontinue:
	inc	edi			; increment pixel buffer pointer.
	shr	bl,1			; shift to next test bit. if shift
	loopnz	#slowloop		; result & counter both non-zero loop,
	jecxz	short #slowdone 	; else if counter is zero, done, else
	mov	bl,80h			; load a fresh bit-test mask, and get
	lodsb				; the next byte from the mask array,
	jmp	short #slowloop 	; then loop to do some more.
#slowdone:
	pop	ebp
	jmp	short mask2exit

pj_vdrv_mask2line endp

;*****************************************************************************
;* pj_vdrv_zoom_line(Pixel *source, Pixel *dest, int swidth, int expand_x);
;*
;*  Expand a line of pixels, replicating each input pixel expand_x times.
;*
;*  Source & dest may NOT be the same buffer!
;*
;*  Output buffer must have room for swidth*expand_x pixels, this routine does
;*  not handle the trucation when swidth is not a multiple of expand_x. (The
;*  caller handles such truncation by not writing the extra pixels).  But the
;*  implication is that the dest buffer had better have enough extra room to
;*  hold the few overflow pixels that get generated here but not written to
;*  the screen by caller.
;*****************************************************************************

pj_vdrv_zoom_line proc near

	Entry
	Args	#source,#dest,#swidth,#expand_x
	Save	esi,edi

	mov	esi,#source		; load input pointer.
	mov	edi,#dest		; load output pointer.
	mov	ecx,#swidth		; load source width.
	mov	edx,#expand_x		; load expansion factor.
	cmp	edx,2			; if expansion factor is one of the
	je	short #loop2		; most common (2,3,4), jump to a
	cmp	edx,3			; routine tuned for that size, else
	je	short #loop3		; fall into the general-purpose
	cmp	edx,4			; expansion routine.
	je	short #loop4		;
#general:
	push	ebx			; save reg used only by general logic.
	mov	ebx,ecx 		; save source width in loop counter.
#loopg:
	lodsb				; get a source pixel.
	mov	ecx,edx 		; load replication factor.
	rep stosb			; replicate the pixel.
	dec	ebx			; decrement source pixel count,
	jnz	short #loopg		; if more pixels, continue loop.
	pop	ebx			; restore reg used only by general
	jmp	short #done		; loop, go exit.
#loop4:
	lodsb				; loop for x4 zoom, get input byte,
	mov	ah,al			; replicate it into ax.
	stosw				; store 2 bytes.
	stosw				; store 2 bytes.
	dec	ecx			; decrement source pixel count,
	jnz	short #loop4		; if more pixels, continue loop,
	jmp	short #done		; else go exit.
#loop3:
	lodsb				; loop for x3 zoom, get input byte,
	mov	ah,al			; replicate it into ax.
	stosw				; store 2 bytes.
	stosb				; store 3rd byte.
	dec	ecx			; decrement source pixel count,
	jnz	short #loop3		; if more pixels, continue loop.
	jmp	short #done		; loop, go exit.
#loop2:
	lodsb				; loop for x2 zoom, get input byte,
	mov	ah,al			; replicate it into ax.
	stosw				; store 2 bytes.
	dec	ecx			; decrement source pixel count,
	jnz	short #loop2		; if more pixels, continue loop.
#done:
	Restore esi,edi
	Exit

pj_vdrv_zoom_line endp

;*****************************************************************************
;* pj_vdrv_tblit_line(Pixel *source, Pixel *dest, int width, int tcolor);
;*
;*  Copy a line from source to dest where only non-tcolor pixels are copied.
;*
;*  The logic here is totally straightforward except that a dword at a time
;*  is loaded from the source, and the output loop is 'unrolled'.
;*****************************************************************************

pj_vdrv_tblit_line proc near

	Entry
	Args	#source,#dest,#width,#tcolor
	Save	esi,edi

	mov	esi,#source
	mov	edi,#dest
	mov	ecx,#width
	mov	dl,#tcolor
	align 4
#loop:
	lodsd				; get next four bytes of input
	cmp	al,dl			; is next byte transparent color?
	je	short #nostore1 	; yep, skip it.
	mov	[edi],al		; nope, write it.
#nostore1:
	inc	edi			; incr output pointer.
	dec	ecx			; decr loop counter.
	jz	short #done		; jump if done.
	shr	eax,8			; get next input byte.
	cmp	al,dl			; is next byte transparent?
	je	short #nostore2 	; you get the idea...
	mov	[edi],al
#nostore2:
	inc	edi
	dec	ecx
	jz	short #done
	shr	eax,8
	cmp	al,dl
	je	short #nostore3
	mov	[edi],al
#nostore3:
	inc	edi
	dec	ecx
	jz	short #done
	shr	eax,8
	cmp	al,dl
	je	short #nostore4
	mov	[edi],al
#nostore4:
	inc	edi
	dec	ecx
	jnz	short #loop
#done:
	Restore esi,edi
	Exit

pj_vdrv_tblit_line endp

;*****************************************************************************
;* pj_vdrv_put_rectram(Bytemap *raster, Pixel *pixbuf, int x,y,w,h);
;*
;*  This is just like put_rectpix, except the destination is a Bytemap raster.
;*****************************************************************************

pj_vdrv_put_rectram proc near

	Entry
	Args	#raster,#pixbuf,#x,#y,#w,#h
	Save	ebx,esi,edi

	mov	edx,#raster
	mov	eax,[edx].bym_bpr	; get bytes-per-row for the bytemap,
	mov	edi,eax 		; subtract bpr for hardware, giving
	sub	eax,#w			; deltabpr: distance from end of this
	imul	edi,dptr #y		; this move to start of next move.
	add	edi,#x			; calculate the starting location in
	add	edi,[edx].bym_p 	; the bytemap as (bpr*y+x+bytemap).

	mov	esi,#pixbuf		; get pointer to our source buffer.
	mov	ebx,#w			; the width to be moved for each line.
	mov	edx,#h			; the number of lines to do.
#loop:
	mov	ecx,ebx 		; typical fastmove stuff...
	shr	ecx,2
	rep movsd
	mov	cl,bl
	and	cl,3
	rep movsb
	add	edi,eax
	dec	edx			; decr line counter,
	jnz	#loop			; loop until all lines done.

	Restore ebx,esi,edi
	Exit

pj_vdrv_put_rectram endp

;*****************************************************************************
; void blit_to_ram(Raster  *srast, Coor sx, Coor sy,
;		   Bytemap *drast, Coor dx, Coor dy,
;		   Coor dw, Coor dh);
;
;  Blit from hardware raster to a ram raster.
;  We repeatedly call our get_seg routine; on each call we pass as the buffer
;  pointer a pointer to the appropriate location within the bytemap.
;*****************************************************************************

pj_vdrv_blit_to_ram proc near

	Entry
	Lclvars #deltabpr
	Args	#srast,#sx,#sy,#drast,#dx,#dy,#dw,#dh
	Save	ebx,esi,edi

	mov	edx,#drast		; load dest raster pointer,
	mov	eax,[edx].bym_bpr	; calc delta bpr to be added at the
	mov	edi,eax 		; end of each line to point to the
	sub	eax,#dw 		; next line in the bytemap...
	mov	#deltabpr,eax
	imul	edi,dptr #dy		; calc starting location in bytemap...
	add	edi,#dx
	add	edi,[edx].bym_p

	mov	ebx,#sy
	lea	edx,pj_vdrv_wcontrol
#loop:
	mov	eax,ebx 		; set up call to internal (fast)
	mov	ecx,#dw 		; entry point to our get_seg routine...
	mov	esi,#sx
	call	_pj_vdrv_get_hseg	       ; move seg from hardware to bytemap.
	inc	ebx			; incr line number.
	add	edi,#deltabpr		; adjust bytemap pointer to next line
	dec	dptr #dh		; in bytemap.
	jnz	#loop			; loop until all lines done.

	Restore ebx,esi,edi
	Exit

pj_vdrv_blit_to_ram endp

;*****************************************************************************
; void blit_from_ram(Bytemap *srast, Coor sx, Coor sy,
;		     Raster  *drast, Coor dx, Coor dy,
;		     Coor dw, Coor dh);
;
; Blit from ram raster to hardware raster.
; This works just like the blit_to_ram above, except we call our put_seg
; routine repeatedly.
;*****************************************************************************

pj_vdrv_blit_from_ram proc near

	Entry
	Lclvars #deltabpr
	Args	#srast,#sx,#sy,#drast,#dx,#dy,#dw,#dh
	Save	ebx,esi,edi,es

	mov	ax,gs
	mov	es,ax

	mov	edx,#srast
	mov	eax,[edx].bym_bpr
	mov	esi,eax
	sub	eax,#dw
	mov	#deltabpr,eax
	imul	esi,dptr #sy
	add	esi,#sx
	add	esi,[edx].bym_p

	mov	ebx,#dy
	lea	edx,pj_vdrv_wcontrol
#loop:
	mov	eax,ebx
	mov	ecx,#dw
	mov	edi,#dx
	call	_pj_vdrv_put_hseg
	inc	ebx
	add	esi,#deltabpr
	dec	dptr #dh
	jnz	#loop

	Restore ebx,esi,edi,es
	Exit

pj_vdrv_blit_from_ram endp

;*****************************************************************************
;*
;*****************************************************************************

pj_vdrv_memcpy proc near

	Entry
	Args	#dst,#src,#count
	Save	esi,edi

	mov	esi,#src
	mov	edi,#dst
	mov	ecx,#count
	mov	eax,ecx
	shr	ecx,2
	rep movsd
	mov	cl,al
	and	cl,3
	rep movsb

	Restore esi,edi
	Exit

pj_vdrv_memcpy endp

_TEXT	ends
	end


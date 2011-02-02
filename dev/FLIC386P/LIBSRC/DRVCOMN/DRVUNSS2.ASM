;*****************************************************************************
;* DRVSS2.ASM - The unss2 fli decompression/playback driver function.
;*
;*  NOTES:
;*		See notes in function header below.
;*
;*  MAINTENANCE:
;*    03/27/91	Ian Lepore
;*		Basically a total re-write.
;*    04/20/91	Ian
;*		Fixed a bug in the last-odd-pixel call setup.  The output X
;*		coor was calc'd as x+width, now it's x+width-1.
;*    04/25/91	Ian
;*		Did a couple minor speed tweaks based on things learned when
;*		coding the unbrun/unlccomp routines.  Managed to shave about
;*		3-4 cycles off each iteration of the inner loop, but also
;*		managed to substitute some shorter instructions in a few
;*		places.  Also some instructions were shuffled to give a better
;*		short-long-short-long-instruction sequence in a few places;
;*		hopefully, this will help keep the prefetch running full.
;*    06/22/91	Ian
;*		Fixed the handling of split lines.  Originally, to avoid a
;*		lot of messy special-case code, split lines were handled by
;*		parsing out the packets and calling put_hseg or set_hline to
;*		do the output.	The problem is, set_hline makes runs of bytes,
;*		not runs of words, so it failed horribly. <embarrassed grin>
;*		Now we have a lot of messy special-case code for handling
;*		split lines rather than calling out to other routines.
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

_TEXT	segment

	extrn	pj_vdrv_put_dot:near
	extrn	pj_vdrv_put_hseg:near
	extrn	pj_vdrv_set_hline:near
	public	pj_vdrv_unss2

;*****************************************************************************
;* void unss2_rect(Hrast *r, void *psource, long pixsize, Coor x,y,w,h);
;*
;*  Notes:
;*	 There is a trickiness in testing the two high bits of the line op
;*	 word in the code below.  These bits can have the following values:
;*	   00 - normal op: low 14 bits hold line delta packet count.
;*	   01 - can't happen (unless there are 2^14 line deltas: not possible).
;*	   10 - special op: output (op&0x00FF) as the last pixel on the line.
;*	   11 - skip count: add (-op) to current y position.
;*	 The trickiness lies in using a 1-bit left shift to test the state of
;*	 both of these bits at once.  The left shift will set the carry flag
;*	 to the value of the high bit, and will set the overflow flag if the
;*	 two highest bits are not equal.  Since we have defined '01' as a bit
;*	 sequence that can't happen, the only thing that can set the overflow
;*	 flag is the '10' sequence, so we jump to the special last-pixel
;*	 routine if the shift sets the overflow flag.  If the overflow flag is
;*	 not set, we check the carry flag.  If it is set, we have '11' as the
;*	 two high bits, and thus jump to the skiplines routine.
;*
;*	 In several places within this routine, instructions are placed in
;*	 a sort of arbitrary order (eg, load linenum, zero a register, load
;*	 ytable using linenum, zero a register, compare loaded ytable value,
;*	 etc).	Normally you would group the zero-a-register instructions,
;*	 the line/ytable-related instructions, etc, for readability.  Where
;*	 the instruction sequence seems a bit arbitrary, it has been done
;*	 to keep a mix of short-operand and long-operand instructions in
;*	 order, hopefully allowing the 386 prefetch buffer to stay full.
;*****************************************************************************

	align 4
pj_vdrv_unss2 proc	near

	Entry				; setup local vars and passed args...
	Lclvars #linecnt		; name the local vars
	Args	#rast,#psource,#pixsize,#xcoor,#ycoor,#wcoor,#hcoor ;name args
	Save	ebp,ebx,esi,edi,es	; alloc local vars, save registers.

	mov	ax,gs			; point ES to video/dos memory for
	mov	es,ax			; stos/movs instructions.

	mov	esi,#psource		; load pointer to compressed data.
	xor	eax,eax 		; clean out high order eax,
	mov	ebx,#ycoor		; load starting Y coordinate,
	lodsw				; load line counter,
	mov	#linecnt,eax		; save it in our local variable.
	lea	edx,pj_vdrv_wcontrol		; load pointer to window control table
	jmp	short lineloop		; jump to start of processing

done:
	Restore ebp,ebx,esi,edi,es	; deallocate local vars & restore regs.
	Exit				; return to caller.

	align 4
odd_pixel:				;output last pixel on odd-width line...
	mov	ecx,#xcoor		; last pixel on a line will be at
	dec	ecx			; x+width-1, calculate that location.
	add	ecx,#wcoor
	push	ebx			; push y
	push	ecx			; push x
	push	ebp			; push pixel
	sub	esp,4			; our put_dot doesn't need raster ptr.
	call	pj_vdrv_put_dot	       ; call dot output.
	add	esp,16			; clean up stack parms.  refresh edx
	lea	edx,pj_vdrv_wcontrol		; reg that put_dot may have trashed.
	jmp	short lineloop		; continue with next line delta op.

	align 4
nextline:
	dec	dptr #linecnt	    ;6	; decrement line packet counter, if
	jz	short done	    ;3	; all done, exit. (ebp always -1 here).
skiplines:			    ;
	sub	ebx,ebp 	    ;2	; skip value is negative; this is an add.
lineloop:			    ;
	lodsw			    ;5	; get next line op from data stream.
	movsx	ebp,ax		    ;3	; save op to packetcount/skiplinecount
	shl	ax,1		    ;3	; register.  test two high bits of the
	jo	short odd_pixel     ;3	; op with a leftshift.	if the high
	jc	short skiplines     ;3	; bits indicate special op, go do it.

	mov	edi,#xcoor	    ;4	; load the starting x coordinate, add
	add	edi,[ebx*8+ytab_offs];6	; the video memory offset for this line.
	xor	ecx,ecx 	    ;2	; (blast ecx high bits for packet loop)
	mov	eax,[ebx*8+ytab_bank]  ;4	; compare the split/bank value for this
	cmp	eax,[edx].wwrcurbank;6	; line to the current hdw bank. if not
	jne	short checksplit    ;3	; equal, go do splitline or bankswap.
nosplit:
	add	edi,[edx].wwraddr   ;6	; add the base video write address.
packetloop:
	dec	ebp		    ;2	; decrement packet count, if done with
	js	short nextline	    ;3	; packets, go get next line op word.

	lodsw			    ;5	; get the X-skip and run/literal length.

	mov	cl,al		    ;2	; ecx is always 0 here; 1st use it to
	add	edi,ecx 	    ;2	; clean up the skip count, add skip to
	mov	cl,ah		    ;2	; output ptr, then mov in run/lit len.
data_after_split:
	test	cl,cl		    ;2	; have a run? test the op for negative.
	js	short dorun	    ;3	; if count is negative, go do the run.
	shr	ecx,1		    ;3	; change word count to dword count.
	rep movsd			; move the literal to video memory.
	jnc	short packetloop	; in case word count was odd, move
	movsw				; the last word.
	jmp	short packetloop	; go do the next delta packet.

	align 4
dorun:
	neg	cl		    ;2	; make literal count positive again,
	lodsw			    ;5	; load the word to be repeated.
	rep stosw		    ;	; store the run.
	jmp	short packetloop    ;	; go do the next delta packet.

;-----------------------------------------------------------------------------
; bank switch and split line handling...
;-----------------------------------------------------------------------------

	align 4
checksplit:
	test	eax,0FFFF0000h		; split line or just bank swap?
	jnz	short splitline 	; split lines get special handling.
	SetWriteBank			; if not split, just change to the
	jmp	short nosplit		; new bank, then continue as normal.

	align 4
splitline:

	cmp	edi,[edx].woffsmask	; if 1st X is less than split point
	jle	short truesplit 	; we have a true split, else we'll
	and	eax,0000FFFFh		; be doing all output after the split
	inc	eax			; point, so just increment to the next
	and	edi,[edx].woffsmask	; bank number (ie, the bank after the
	SetWriteBank			; split), set that bank, then continue
	jmp	nosplit 		; as if there were no split.

splitdone:
	pop	ebx
	jmp	nextline

truesplit:
	mov	ecx,eax 		; make ecx equal the number of bytes
	shr	ecx,16			; left between our starting X coor
	sub	ecx,#xcoor		; and the split point.

	and	eax,0000FFFFh		; clean out split_at, leaving bank
	cmp	eax,[edx].wwrcurbank	; number for line, compare to current
	je	short #splitbankokay	; bank, if needed, go set new
	SetWriteBank			; write bank.
#splitbankokay:
	push	ebx			; save ebx, we use it for tracking
	mov	ebx,ecx 		; the split point.  move bytes-until
	add	edi,[edx].wwraddr	; split into ebx, fix video pointer.
	xor	ecx,ecx 		; clean out ecx for the packet loop.

	align 4
#splitploup:				 ;packet loop for split lines...
	dec	ebp			; decrement packet count, if done with
	js	short splitdone 	; packets, go get next line data.

	lodsw				; get the X-skip and run/literal length.

	mov	cl,al			; ecx is always 0 here; clean up skip
	add	edi,ecx 		; skip count, add skip to o/p ptr, sub
	sub	ebx,ecx 		; from bytes left before skip.	move
	mov	cl,ah			; copy/run length to count reg. if the
	jns	short #notskippastsplit ; skip took us into the new bank,
	and	edi,[edx].woffsmask	; mask addr, leaving offset into new
	add	edi,[edx].wwraddr	; bank, then add back the video address.
	mov	eax,[edx].wwrcurbank	; pull a typical bankswitch to get
	inc	eax			; into the bank following the one
	SetWriteBank			; we were just writing to, then pop
	pop	ebx			; ebx and continue in the main loop
	jmp	data_after_split	; since the rest of the line is unsplit.

#notskippastsplit:

	neg	cl			; have a run? neg the op to a runlength.
	js	short #splitlit 	; if count now negative, we have literal.

	lodsw				; load the word to be repeated.
	sub	ebx,ecx 		; subtract length of run (twice because
	sub	ebx,ecx 		; it's a word length) from bytes left
	js	short #splitrun 	; before split. go do special handling
	rep stosw			; if this run crosses the split itself,
	jmp	short #splitploup	; else store the run and continue.

#splitrun:
	lea	ebx,[ebx+ecx*2] 	; add len back for word-at-a-time work.
#splitrunloop:
	sub	ebx,2			; subtract another word from the bytes
	js	short #runpastsplit	; remaining before the split, if we
	stosw				; go negative, we are at the split,
	dec	ecx			; else store the word, decr the count,
	jz	#splitploup		; and loop if length remains in the
	jmp	#splitrunloop		; run, else go do the next packet.

	align 4
#runpastsplit:
	test	edi,1			; if we're odd-aligned we need special
	jnz	short #oddsplitrun	; handling, else we're at the first
	mov	edi,[edx].wwraddr	; word in the new bank, so mask off
	mov	ebx,eax 		; the video address accordingly.
	mov	eax,[edx].wwrcurbank	; stash the run word for a mo, and
	inc	eax			; do a bankswitch into the bank
	SetWriteBank			; following the one we've been writing
	mov	eax,ebx 		; to.  restore the run word into eax
	rep stosw			; and store the rest of the run.  then
	pop	ebx			; pop ebx and jump into the main loop
	jmp	packetloop		; above to finish (now unsplit) line.

#oddsplitrun:
	mov	gs:[edi],al		; special handling for odd-aligned
	mov	edi,[edx].wwraddr	; lines, we have to write the last
	mov	ebx,eax 		; byte in the current bank from the
	mov	eax,[edx].wwrcurbank	; low byte of the run word.  then we
	inc	eax			; pull a normal bankswitch just like
	SetWriteBank			; above.  after the switch, we have
	mov	eax,ebx 		; to write the first byte from the
	mov	gs:[edi],ah		; high byte of the run word (note edi
	inc	edi			; was masked to even addr above). we
	dec	ecx			; incr edi to get odd-aligned again,
	rep stosw			; decr the count, then we can store
	pop	ebx			; the remainder of the run, then go
	jmp	packetloop		; finish the line in the normal loop.

	align 4
#splitlit:
	neg	cl			; make literal count positive again,
	sub	ebx,ecx
	sub	ebx,ecx
	js	short #splitisinlit
	rep movsw
	jmp	#splitploup




	align 4
#splitisinlit:
	lea	ebx,[ebx+ecx*2]
#splitlitloop:
	sub	ebx,2
	js	short #litpastsplit
	movsw
	dec	ecx
	jz	#splitploup
	jmp	#splitlitloop

	align 4
#litpastsplit:
	test	edi,1			; if we're odd-aligned we need special
	jnz	short #oddsplitlit	; handling, else we're at the first
	mov	edi,[edx].wwraddr	; word in the new bank, so mask off
	mov	eax,[edx].wwrcurbank	; the video address accordingly.
	inc	eax			; do a bankswitch into the bank
	SetWriteBank			; following the one we've been writing
	rep movsw			; to.  store the rest of the run.  then
	pop	ebx			; pop ebx and jump into the main loop
	jmp	packetloop		; above to finish (now unsplit) line.

#oddsplitlit:
	movsb				; special handling for odd-aligned
	mov	edi,[edx].wwraddr	; lines, we have to write the last
	mov	ebx,eax 		; byte in the current bank from the
	mov	eax,[edx].wwrcurbank	; next byte of the source buf.	then we
	inc	eax			; pull a normal bankswitch just like
	SetWriteBank			; above.  after the switch, we have
	mov	eax,ebx 		; to write the first byte from the
	movsb				; next byte of the source buf (note edi
	dec	ecx			; was reset to even addr above).
	rep movsw			; decr the count, then we can store
	pop	ebx			; the remainder of the run, then go
	jmp	packetloop		; finish the line in the normal loop.

pj_vdrv_unss2 endp

_TEXT	ends
	end

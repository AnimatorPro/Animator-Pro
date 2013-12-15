DEBUG = 1 ; define to include debugging that will crash on zero-count copy/run.

;*****************************************************************************
;*
;*****************************************************************************

	include stdmacro.i

_BSS	segment

COPYBREAK    equ 2
RUNBREAK     equ 3
RUNINCOPY    equ 4

SS2Ctl	struc

remaining    dd ?			; width of words remaining during deltas.

bwidth	     dd ?			; width in bytes, only if odd width.
wwidth	     dd ?			; width of lines in words.
oldline      dd ?			; pointer to old line data.
newline      dd ?			; pointer to new line data.
compbuf      dd ?			; pointer to compression buffer.

lskip	     dd ?			; accumulated lineskip value.
lines	     dd ?			; count of lines with changes.
packets      dd ?			; count of packets on current line.
plinecount   dd ?			; point to patch addr for line count.
ppacketcount dd ?			; pointer to patch addr for packet count.

skipbase     dd ?			; pointer to base at which skips start.
skipcount    dd ?			; word skip count, used by write_skips
runcount     dd ?			; run count for current delta

SS2Ctl	ends

ss2_ctl SS2Ctl	<>			; define an instance of it.

_BSS   ends

_TEXT	segment

	public	pj_ss2line
	public	pj_ss2line_start
	public	pj_ss2line_finish

      ifdef DEBUG
	public	__make_skips		; wvideo likes global names...
	public	_save_copy
	public	_save_run
	public	_write_lineskip
      endif

;*****************************************************************************
;* void pj_ss2line_start(void *cbuf, int width)
;*
;*   set up for SS2'ing a pair of buffers.
;*
;*   this takes the parms from our caller which are constant during the
;*   processing of the buffers and stuffs them into our local control structure
;*   (which is not visible to the outside).  it also zeroes out some other
;*   parts of the structure, and pre-calcs some values needed for the
;*   last-odd-pixel processing when the line width is odd.
;*****************************************************************************

pj_ss2line_start proc near

	Entry
	Args	#compbuf,#width

	lea	edx,ss2_ctl		; load a pointer to our structure.

	xor	eax,eax 		; blast the following fields...
	mov	[edx].lskip,eax
	mov	[edx].lines,eax
	mov	[edx].bwidth,eax

	mov	eax,#compbuf		; get the compression buffer ptr, save
	mov	[edx].plinecount,eax	; it as the patch location for filling
	add	eax,2			; in the line-ops count later, increment
	mov	[edx].compbuf,eax	; past the count and save the pointer.

	mov	eax,#width		; get the width in bytes, adjust
	shr	eax,1			; it to be the width in words,
	mov	[edx].wwidth,eax	; and save it.
	jnc	short #done		; if the width was even, we're done.

	lea	eax,[eax*2+1]		; make width back into byte count.
	mov	[edx].bwidth,eax

#done:
	Exit

pj_ss2line_start endp

;*****************************************************************************
;* USHORT *pj_ss2line_finish(void)
;*   patches the line count into the first word of the buffer, and returns
;*   the current (ie, ending) pointer into the compression buffer.  if no
;*   lines had changes, the line count is patched to 0, but NULL is returned.
;*****************************************************************************

pj_ss2line_finish proc near

	Entry

	lea	edx,ss2_ctl
	mov	eax,[edx].lines
	mov	ecx,[edx].plinecount
	mov	[ecx],ax
	test	eax,eax
	jz	short #done
	mov	eax,[edx].compbuf
#done:
	Exit

pj_ss2line_finish endp

;*****************************************************************************
;* internal subroutines for the ss2line process...
;*****************************************************************************

;-----------------------------------------------------------------------------
; _write_lineskip - dump pending linkskip value and setup for making packets.
;
;   Entry:
;     values in ss2_ctl (pointed to by ebx) are valid.
;     no parms passed in regs.
;   Exit:
;     ebp,edx  = trashed
;     (note to self: callers count on eax, ecx preserved)
;-----------------------------------------------------------------------------

	align 4
_write_lineskip proc near

	mov	edx,[ebx].compbuf	; get the lineskip count, if it is
	mov	ebp,[ebx].lskip 	; non-zero we have to write it to the
	test	ebp,ebp 		; output buffer before the line deltas.
	jz	short #noskiplines	; after writing it, increment the
	mov	[edx],bp		; pointer to account for the word
	add	edx,2			; we wrote, and zero out the
	xor	ebp,ebp 		; lineskip count.
	mov	[ebx].lskip,ebp
#noskiplines:
	mov	[ebx].packets,ebp	; zero out packet count, save pointer to
	mov	[ebx].ppacketcount,edx	; patch location into which packet count
	add	edx,2			; stored later, increment the output
	mov	[ebx].compbuf,edx	; to the word after the packet count,
	ret				; save the pointer, and return.

_write_lineskip endp

;-----------------------------------------------------------------------------
; __make_skips - must be called ONLY from _save_copy or _save_run!
;
;  Entry:
;    [ebx].compbuf   -> compression (output) buffer
;    [ebx].skipbase  -> start of skip data in newline
;    [ebx].skipcount =	the number of skips to put before the literal
;  Exit:
;    eax	     = remaining skipcount (goes into next packet)
;    edx	     = trashed
;    esi	     -> start of delta data
;    edi	     -> next word in compression buffer
;    [ebx].skipcount =	zero
;-----------------------------------------------------------------------------

	align 4
__make_skips proc near

	mov	esi,[ebx].skipbase	; load pointer to start of skip data.
	mov	edi,[ebx].compbuf	; load pointer to compression buffer.
	xor	edx,edx 		; blast edx, then load skipcount into
	xchg	[ebx].skipcount,edx	; edx and zero out skipcount.
	mov	ax,01FEh
#skiploop:
	cmp	edx,127
	jle	short #smallskip
	inc	[ebx].packets
	add	esi,254
	stosw
	movsw
	sub	edx,128
	jnz	short #skiploop

#smallskip:
	lea	eax,[edx+edx]
	add	esi,eax
	ret

__make_skips endp

;-----------------------------------------------------------------------------
; _save_copy
;  Entry:
;    ecx =  copycount
;    ebp =  curskips: gets subtracted from copycount to get true length
;    [ebx].compbuf   -> compression (output) buffer
;    [ebx].skipbase  -> start of skip data in newline
;    [ebx].skipcount =	the number of skips to put before the literal
;  Exit:
;    ecx zeroed, all others preserved
;    [ebx].compbuf   -> next location in compression buffer
;    [ebx].skipbase  -> new base for skipped data
;    [ebx].skipcount =	zero
;-----------------------------------------------------------------------------

	align 4
_save_copy proc near

	sub	ecx,ebp 		; we'll do copycount-curskips words.
      ifdef DEBUG
	jle	short #crashit		; (just for debugging)
      endif

_run_1_as_copy:

	push	eax
	push	edx
	push	esi
	push	edi

	call	__make_skips		; write long skips, return small skip.

	mov	edx,ecx 		; in case we gotta do pieces, save count.
#copyloop:
	inc	[ebx].packets		; count this packet.
	mov	ecx,127 		; assume count is gonna be big
	cmp	edx,ecx 		; is it a big'un?
	jge	short #docopy		; yep, go do 127 words of it.
	mov	ecx,edx 		; it's small, plug in the real count.
#docopy:
	mov	ah,cl			; put the copycount into the packet
	stosw				; header (skip is already in AL), and
	rep movsw			; store the header, followed by data.
	xor	al,al			; skipcount goes into 1st packet only.
	sub	edx,127 		; subtract off the max packet size,
	jg	short #copyloop 	; if still positive, do some more.

	mov	[ebx].skipbase,esi	; save new base pointer for skip data.
	mov	[ebx].compbuf,edi	; save new location in compression buf.

	pop	edi
	pop	esi
	pop	edx
	pop	eax
	ret

      ifdef DEBUG
#crashit:
	int 3
	mov	eax,[ecx]		; crash and burn: count is 0 or negative.
      endif

_save_copy endp


;-----------------------------------------------------------------------------
; _save_run
;  Entry:
;    ecx =  runcount
;    ebp =  curskips: gets subtracted from runcount to get true length
;    [ebx].compbuf   -> compression (output) buffer
;    [ebx].skipbase  -> start of skip data in newline
;    [ebx].skipcount =	the number of skips to put before the literal
;  Exit:
;    ecx zeroed, all others preserved
;    [ebx].compbuf   -> next location in compression buffer
;    [ebx].skipbase  -> new base for skipped data
;    [ebx].skipcount =	zero
;-----------------------------------------------------------------------------

	align 4
_save_run proc near

	sub	ecx,ebp 		; we'll do runcount-curskips words.
      ifdef DEBUG
	jle	short #crashit		; (just for debugging)
      endif
	cmp	ecx,1			; a hack:  if the run length is just
	je	_run_1_as_copy		; 1 word, go make it a copy instead.

	push	eax
	push	edx
	push	esi
	push	edi

	call	__make_skips		; write long skips, return small skip.

	mov	edx,ecx 		; in case we gotta do pieces, save count.
#runloop:
	inc	[ebx].packets		; count this packet.
	mov	ecx,127 		; assume count is gonna be big
	cmp	edx,ecx 		; is it a big'un?
	jge	short #dorun		; yep, go do 127 words of it.
	mov	ecx,edx 		; it's small, plug in the real count.
#dorun:
	mov	ah,cl			; put the runcount into the packet
	neg	ah			; header, negatize it into a runcount,
	stosw				; (skipcount is already in AL). store
	movsw				; header, followed by the run word.
	xor	al,al			; skipcount goes into 1st packet only.
	lea	esi,[esi+ecx*2-2]	; increment input pointer past rundata.
	sub	edx,127 		; subtract off the max packet size,
	jg	short #runloop		; if still positive, do some more.

	mov	[ebx].skipbase,esi	; save new base pointer for skip data.
	mov	[ebx].compbuf,edi	; save new location in compression buf.

	pop	edi
	pop	esi
	pop	edx
	pop	eax
	ret

      ifdef DEBUG
#crashit:
	int 3
	mov	eax,[ecx]		; crash and burn: count is 0 or negative.
      endif

_save_run endp

;*****************************************************************************
;* void *pj_ss2line(void *oldline, void *newline);
;*****************************************************************************

	align 4
pj_ss2line proc near

	Entry
	Args	#oldline,#newline
	Save	esi,edi

	lea	edx,ss2_ctl

	mov	edi,#oldline		; load pointer to old line data
	mov	esi,#newline		; load pointer to new line data
	mov	[edx].skipbase,esi	; got to save this now, while accessible.

	mov	ecx,[edx].bwidth	; get width of line in bytes,
	test	ecx,ecx 		; if zero, the line width is a
	jz	short #even_width	; multiple of 2, go do it fast.

;-----------------------------------------------------------------------------
; special handling for when the line width is odd...
;   we may need to make a special last-odd-pixel packet, and if we do, we
;   have to exit via FINISH even if the rest of the line is equal, so this
;   little chunk of code (which will almost never get used, I bet) handles
;   all of that.  if the last odd pixels match, we just move down into the
;   the main part of the code; it's only when we've actually created a
;   special packet for the last pixel that we have to exit through FINISH.
;   so, most of the code here duplicates the main compare logic below,
;   except that the ss2_ctl pointer is in ebx while in this compare logic.
;-----------------------------------------------------------------------------

	mov	ah,[esi+ecx]		; get byte from new line, then compare
	cmp	[edi+ecx],ah		; compare it to the oldline byte.
	je	short #even_width	; if equal, go check rest of line.

	push	ebx			; oops, we have to make a last-odd-pix
	push	ebp			; packet, so save and swap the regs to
	mov	ebx,edx 		; the way the packet generators like em.

	call	_write_lineskip 	; go dump pending lineskips.

	mov	al,0C0h 		; put magic value into packet.
	mov	edx,[ebx].compbuf	; load compression buffer pointer,
	sub	edx,2			; compensate for putting packet where
	mov	[edx],ax		; we thought the packet count would be,
	mov	[ebx].ppacketcount,edx	; set the new patch location for the
	add	edx,2			; packet count, leave room for it,
	mov	[ebx].compbuf,edx	; then store the new compbuf pointer.

	mov	ecx,[ebx].wwidth	; get line width in words.
	movzx	eax,cl			; save the low order bit from the width,
	and	al,1			; it indicates presence of extra word.
	shr	ecx,1			; convert to width in dwords.
	repe cmpsd			; compare the lines, if we exit the
	jne	short #makemorepackets	; compare due to a NE condition, go
	dec	al			; make delta packets.  if there's no
	js	short #FINISH		; extra word after all dwords, all done.
	sub	esi,2			; kludge-o-rama dept: back up the ptrs
	sub	edi,2			; so we can compare the last word as
	cmpsd				; part of a dword, since the rest of
	jne	short #makemorepackets	; our logic assumes we were doing
	jmp	short #FINISH		; dwords at the time.

;-----------------------------------------------------------------------------
; the main compare logic...
;   this is the 'normal' line compare routine, for when the line width is
;   even.  the only weirdness here (and it exists above too), has to do with
;   handling lines whose width is not a multiple of 4 (ie, has an extra word).
;   we compare dwords for speed, but if we get into generating packets, those
;   routines assume we were doing dwords, and they adjust the dword count
;   back to a word count, and back up the pointers, and so on. so when we
;   have to check an extra last word on a line that isn't a dword-multiple
;   size, we cheat here and back up the pointers so that the last word is
;   compared as a part of a dword (ie, the second-to-last word gets checked
;   twice).  really, this isn't as slow as it sounds, and it eliminates a
;   ton of special-case handling that used to exist for handling a last word.
;-----------------------------------------------------------------------------

	align 4
#even_width:
	mov	ecx,[edx].wwidth	; get line width in words.
	movzx	eax,cl			; save the low order bit from the width,
	and	al,1			; it indicates presence of extra word.
	shr	ecx,1			; convert to width in dwords.
	repe cmpsd			; compare the lines, if we exit the
	jne	short #makepackets	; compare due to a NE condition, go
	dec	al			; make delta packets.  if there's an
	js	short #return_equal	; extra word after all the dwords,
	sub	esi,2			; back up the pointers so that we can
	sub	edi,2			; compare them as dwords, this keeps
	cmpsd				; our makepackets logic happy, since
	jne	short #makepackets	; it thinks we were doing dwords.
#return_equal:
	dec	[edx].lskip		; if the lines are identical,increment
#done:					; the lineskip counter, and return.
	mov	eax,[edx].compbuf	; return compression buffer pointer.
	Restore esi,edi
	Exit

;-----------------------------------------------------------------------------
; when a difference is found, come here to make delta packets...
;   when the line width is an odd number of bytes, and we created a special
;   last-odd-pixel packet, above, then we find more differences in the bulk
;   of the line, we'll enter at 'makemorepackets'.
;-----------------------------------------------------------------------------

#makepackets:

	push	ebx
	push	ebp
	mov	ebx,edx

	call	_write_lineskip 	; go write pending skiplines value.

#makemorepackets:			; come here to make packets after odd.

	lea	ecx,[ecx*2+eax+2]	; convert remaining width from dwords
	mov	[ebx].remaining,ecx	; count to words count, save it.

	sub	esi,4			; back up the pointers to the words
	sub	edi,6			; that differed (2 bytes extra for edi).

	mov	ebp,[ebx].wwidth	; load the line width, subtract the
	sub	ebp,ecx 		; width remaining, this is curskips.
	jmp	short #SKIPPING

;-----------------------------------------------------------------------------
; FINISH and friends - for ending on a skip, run, or copy.
;-----------------------------------------------------------------------------

#FINISH_WITH_RUN:
	call	_save_run
	jmp	short #FINISH

#FINISH_WITH_COPY:
	call	_save_copy

#FINISH:

	inc	[ebx].lines		; increment lines-with-changes count.
	mov	edx,[ebx].ppacketcount	; load a pointer to the patch location
	mov	eax,[ebx].packets	; for the line we just did, put the
	mov	[edx],ax		; packet count into the patch word.

	mov	edx,ebx 		; put ss2_ctl pointer back in edx.
	pop	ebp
	pop	ebx
	jmp	#done

;-----------------------------------------------------------------------------
; stately machine code...
;  eax =  current newline word
;  edx =  last newline word
;  ebp =  curskips
;  ecx =  copycount, runcount when in RUNNING state.
;  ebx -> SS2Ctl
;  esi -> newline
;  edi -> oldline
;-----------------------------------------------------------------------------
;-----------------------------------------------------------------------------
; SKIPPING state - entered from lotsa places
;-----------------------------------------------------------------------------

	align 4
#SKIPPING:
	dec	[ebx].remaining 	; decrement count of words remaining,
	js	#FINISH 		; if we end in a skip, we're all done.
	inc	ebp			; assume words will match, and count
	add	edi,2			; them as a skip. incr the oldline
	lodsw				; pointer, get the next word from the
	cmp	[edi],ax		; newline, and compare old to new.
	je	short #SKIPPING 	; if they match, loop for next word,
	dec	ebp			; else un-count this word.
	; fall thru

;-----------------------------------------------------------------------------
; DIF1 state - entered via fallthru from SKIPPING, and from RUNNING state.
;-----------------------------------------------------------------------------

#DIF1:

	mov	[ebx].skipcount,ebp	; save curskips to write_skips variable,
	xor	ebp,ebp 		; it'll be included in the next packet.
	lea	ecx,[ebp+1]		; zero out curskips. set copycount to 1.
	mov	[ebx].runcount,ecx	; we currently have a run of 1 word.

	dec	[ebx].remaining 	; decrement count of words remaining,
	js	#FINISH_WITH_COPY	; if done, finish by dumping copy packet.

	mov	edx,eax 		; move current new to lastnew.
	add	edi,2			; increment to next old word.
	lodsw				; get next new word.
	inc	ecx			; we have two words to deal with now.
	cmp	[edi],ax		; are the two the same?
	jne	short #dif1_checkrun	; if so, we have a skip of one word
	inc	ebp			; that's going now.
#dif1_checkrun:
	cmp	edx,eax 		; does lastnew == curnew?  if so, go
	jne	#COPY2			; deal with a potential run.
	inc	[ebx].runcount		; now have a run of 2 words.
	; fall thru

;-----------------------------------------------------------------------------
; RUN2 state - entered only via fallthru from DIF1.
;-----------------------------------------------------------------------------

#RUN2:

	dec	[ebx].remaining 	; decrement count of words remaining,
	js	#FINISH_WITH_RUN	; if done, finish by dumping run packet.

	mov	edx,eax 		; move current new to lastnew.
	add	edi,2			; increment to next old word.
	lodsw				; get next new word.
	inc	ecx			; we have three words to deal with now.
	inc	ebp			; incr curskips, assumes words match.
	cmp	[edi],ax		; do they match?
	je	short #run2_checkrun	; if so, go check for start of run.
	xor	ebp,ebp 		; current word mismatches, zero skips.
#run2_checkrun:
	inc	[ebx].runcount		; incr runcount, assume words match.
	cmp	edx,eax 		; does last newword equal cur newword?
	je	short #RUNNING		; yep, enter RUNNING state.
	test	ebp,ebp 		; any current skips?  if not, just go
	je	#COPYING_NORUN		; to copying state, at norun entry point.
   ;	mov	ecx,[ebx].runcount
   ;	dec	ecx			; oops, they didn't match, uncount it.
	call	_save_run		; save off the little run, then resume
	jmp	#SKIPPING		; a skipping state.

;-----------------------------------------------------------------------------
; RUNNING state - entered from RUN2 or COPYING states
;-----------------------------------------------------------------------------

#RUNNING:
	mov	ecx,[ebx].runcount	; in this state, runcount is reg'ized.
	jmp	short #running_loop

	align 4
#running_zeroskips:
	xor	ebp,ebp

#running_loop:

	dec	[ebx].remaining 	; decrement count of words remaining,
	js	#FINISH_WITH_RUN	; if done, finish by dumping run packet.

	mov	edx,eax 		; move current new to lastnew.
	add	edi,2			; increment to next old word.
	lodsw				; get next new word.
	cmp	edx,eax 		; does the run continue?
	jne	short #running_endrun	; nope, go end the run.
	inc	ecx			; yep, increment the runcount.
	inc	ebp			; incr curskips, assume words match.
	cmp	[edi],ax		; do they match?
	jne	short #running_zeroskips; if not, go zero out curskips.
	cmp	bp,RUNBREAK		; do we have a worthwhile break in the
	jne	short #running_loop	; run?	if not, go run some more.
	call	_save_run		; if so, save the run,
	jmp	#SKIPPING		; and go to skipping state.

#running_endrun:
	call	_save_run		; break in the run, save what we've
	cmp	[edi],ax		; got, then compare old to new.  if
	jne	#DIF1			; not equal, enter dif1 state, else
	inc	ebp			; increment curskips and enter
	jmp	#SKIPPING		; skipping state.

;-----------------------------------------------------------------------------
; COPY2 state - entry only from DIF1
;-----------------------------------------------------------------------------

#COPY2:

	dec	[ebx].remaining 	; decrement count of words remaining,
	js	#FINISH_WITH_COPY	; if done, finish by dumping copy packet.

	mov	edx,eax 		; move current new to lastnew.
	add	edi,2			; increment to next old word.
	lodsw				; get next new word.
	inc	ecx			; we have three words to deal with now.
	inc	ebp			; incr curskips, assumes words match.
	cmp	[edi],ax		; do they match?
	jne	short #copy2_zeroskips	; if not, zero out curskips & continue.
	cmp	bp,COPYBREAK		; and see if we COPYBREAK skips in a
	jne	short #copy2_checkrun	; row.	if not, just continue. if so,
	call	_save_copy		; save the copy (which'll be 1 word),
	jmp	#SKIPPING		; then enter SKIPPING state.

#copy2_zeroskips:
	xor	ebp,ebp 		; current word mistmatches, zero skips.
#copy2_checkrun:
	cmp	edx,eax 		; does last newword equal cur newword?
	jne	short #COPYING_NORUN	; nope, just continue.
	inc	[ebx].runcount		; yep, count it as a run of 2.
	jmp	short #COPYING

;-----------------------------------------------------------------------------
; COPYING state - entered from COPY2 and RUN2 states.
;-----------------------------------------------------------------------------

#COPYING_NORUN:
	mov	[ebx].runcount,1

#COPYING:

	dec	[ebx].remaining 	; decrement count of words remaining,
	js	#FINISH_WITH_COPY	; if done, finish by dumping copy packet.

	mov	edx,eax 		; move current new to lastnew.
	add	edi,2			; increment to next old word.
	lodsw				; get next new word.
	inc	ecx			; now have another word to deal with.
	cmp	[edi],ax		; are these two the same?
	jne	short #copying_zeroskips; if not, zero out curskips & continue.
	inc	ebp			; if they are equal, count the skip
	cmp	bp,COPYBREAK		; and see if we COPYBREAK skips in a
	jne	short #copying_checkrun ; row.	if not, just continue. if so,
	call	_save_copy		; save the copy, and
	jmp	#SKIPPING		; enter SKIPPING state.

#copying_zeroskips:
	xor	ebp,ebp 		; current word mistmatches, zero skips.
#copying_checkrun:
	cmp	edx,eax 		; does last newword equal cur newword?
	jne	short #COPYING_NORUN	; nope, go copy some more.

	inc	[ebx].runcount		; count the run word.
	cmp bptr[ebx].runcount,RUNINCOPY; are we at the run-embedded-in-copy
	jne	short #COPYING		; break length yet? if not, continue,
	push	ebp			; save curskips, then make the runcount
	mov	ebp,[ebx].runcount	; the curskips value during _save_copy.
	call	_save_copy		; if so, save off the current copy,
	pop	ebp			; restore curskips to its register
	jmp	#RUNNING		; and go do some running.

pj_ss2line endp


_TEXT	ends
	end

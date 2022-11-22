;*****************************************************************************
;* POCLOSEC.ASM - closestc and color_diff routines for COLORUTL POE module.
;*
;*	This version copes with IRgb3 color values (the r,g,b components
;*	are 32bit int, not unsigned byte).  Other than that, it's pretty
;*	much a clone of the internal PJ routines of the same name.
;*
;*	!!! DO NOT LET THIS CODE FIND ITS WAY INTO PJ !!!
;*****************************************************************************

	include stdmacro.i

_DATA	segment

;-----------------------------------------------------------------------------
; the squares table - for negative and positive numbers.
;  (we keep both negatives and positives to avoid needing a branching
;  instruction inside the loops to turn negative diffs into positive).
;-----------------------------------------------------------------------------

	align 4
	IDX=-255			; starting index
sqrneg:
	rept 255
	dd	IDX*IDX 		; squares of numbers from -255 thru -1
	IDX=IDX+1
	endm
sqrtab: 				; index into table is from this point
	rept 256
	dd	IDX*IDX 		; squares of numbers from 0 thru 255
	IDX=IDX+1
	endm

_DATA	ends

_TEXT	segment

	public	color_dif
	public	closestc

;*****************************************************************************
;* int color_dif(IRgb3 *pcolor1, IRgb3 *pcolor2)
;*
;*  return the sum of the squares of the differences of the rgb components.
;*****************************************************************************

	align 4
color_dif proc near

	Entry
	Args	#pcolor1,#pcolor2
	Save	esi,edi

	mov	esi,#pcolor1		; load pointer to color 1
	xor	edx,edx 		; clean out edx high order bits.
	mov	edi,#pcolor2		; load pointer to color 2

	mov	dl,bptr [esi]		; load red byte of color 1
	movzx	ecx,bptr [edi]		; load red byte of color 2
	sub	ecx,edx 		; subtract them
	mov	eax,dptr [ecx*4+sqrtab] ; load the square of the difference

	mov	dl,bptr [esi+4] 	; load green byte of color 1
	movzx	ecx,bptr [edi+4]	; load green byte of color 2
	sub	ecx,edx 		; subtract them
	add	eax,dptr [ecx*4+sqrtab] ; accumulate square of the difference

	mov	dl,bptr [esi+8] 	; load blue byte of color 1
	movzx	ecx,bptr [edi+8]	; load blue byte of color 2
	sub	ecx,edx 		; subtract them
	add	eax,dptr [ecx*4+sqrtab] ; accumulate square of the difference

	Restore esi,edi 		; all done, return the sum of the
	Exit				; squares of the differences.

color_dif endp

;*****************************************************************************
;* int closestc(IRgb3 *pcolor, IRgb3 *ptab, int tabcount)
;*
;*   return the index of the closest match of an IRgb3 color within an
;*   arbitrary-length table of IRgb3 colors.  a zero-length table is legit
;*   (FWIW), and results in a return value of 0.
;*****************************************************************************

	align 4
closestc proc	near

	Entry
	Lclvars #current,#closest
	Args	#pcolor,#ptab,#tabcount
	Save	ebx,esi,edi,ebp

	xor	eax,eax 		; zero out #closest (return value)
	mov	#closest,eax		; and set #current (table index)
	dec	eax			; to -1 (it will get incr'd to zero
	mov	#current,eax		; the first time through the loop).

	mov	eax,#pcolor		; load pointer to color to be matched.
	movzx	ebx,bptr [eax]		; registerize the red, green, and
	movzx	ecx,bptr [eax+4]	; blue components as 32bit values,
	movzx	edx,bptr [eax+8]	; since they are refferred to a lot.
	mov	esi,#ptab		; load pointer to color table.
	mov	edi,7FFFFFFFh		; prime closeness-so-far to high value.
	align 4
#loop:
	inc	dptr #current		; incr current-slot-in-table index.
	dec	dptr #tabcount		; decr loop counter, when it goes
	js	short #done		; negative we're all done.

	movzx	eax,bptr [esi]		; get red from current table slot,
	sub	eax,ebx 		; subtract it from match-color red,
	mov	ebp,dptr [eax*4+sqrtab] ; get square of the difference.
	movzx	eax,bptr [esi+4]	; get green from current table slot,
	sub	eax,ecx 		; subtract it from match-color green,
	add	ebp,dptr [eax*4+sqrtab] ; accumulate square of the difference.
	movzx	eax,bptr [esi+8]	; get blue from current table slot,
	sub	eax,edx 		; subtract it from match-color blue,
	add	ebp,dptr [eax*4+sqrtab] ; accumulate square of the difference.
	add	esi,12			; incr table pointer for next time.

	cmp	edi,ebp 		; if closest-diff-so-far is less than
	jbe	short #loop		; diff we just calc'd, try next color,
	mov	eax,#current		; else we have closer color, remember
	mov	#closest,eax		; its index as the new closest color.
	test	ebp,ebp 		; if the difference is zero (exact
	jz	short #done		; match) we can't do better, get out.
	mov	edi,ebp 		; not an exact match, remember the new
	jmp	short #loop		; closest-diff-so-far and keep trying.
#done:
	mov	eax,#closest		; load index of closest color we found
	Restore ebx,esi,edi,ebp 	; for return to caller.
	Exit

closestc endp

_TEXT	ends
	end

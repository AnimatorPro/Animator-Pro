;*****************************************************************************
;* CMTBLEND.ASM - The true_blend rgb color mixing routine.
;*****************************************************************************

	include stdmacro.i
	assume	cs:CGROUP,ds:DGROUP

_TEXT	segment

	public	true_blend

;*****************************************************************************
;* void true_blend(Rgb3 *src1, Rgb3 *src2, int percent, Rgb3 *dest)
;*
;*   this routine blends the src1 and src2 colors based on the percent,
;*   yielding a new color in dest.  The algorithm is:
;*
;*	UBYTE vpercent = 100-percent;
;*	dest->r = (scr1->r * vpercent + src2->r * percent + 50)/100;
;*	dest->g = (scr1->g * vpercent + src2->g * percent + 50)/100;
;*	dest->b = (scr1->b * vpercent + src2->b * percent + 50)/100;
;*
;*   however, we take some shortcuts with the 386 instruction set...
;*
;*   it is known that percent is a number from 0-100 inclusive (it had
;*   better be, or we die bwana).  we know that the rgb color values are
;*   unsigned bytes.  thus, the result of multiplying an rgb component by
;*   either percent or vpercent has to fit in 16 bits, so we use the fastest
;*   386 multiply, the byte-operand unsigned MUL.  also, since the two
;*   multipliers (percent and vpercent) are proportional to each other,
;*   the results of the two multiplies can be added together, and still
;*   fit in 16 bits (and there's room for the extra 50 we add in too).  for
;*   example, 255 * 100 + 255 * 0 + 50 should be the biggest value we can
;*   get, and it fits in 16 bits (and doesn't look negative).  anyway, since
;*   we know the intermediate result fits in sixteen bits, we can also use
;*   the fastest 386 divide, the word-operand unsigned DIV.  this gives an
;*   8-bit result (and a remainder which we ignore), which is what we want.
;*   other than coding to the pre-knowledge of the range of values we're
;*   dealing with there is nothing tricky about this code, it's just a
;*   brute-force implementation of the lines listed above.
;*
;*****************************************************************************

true_blend proc near

	Entry
	Args	#src1,#src2,#percent,#dest
	Save	esi,ebx,edi,ebp

	mov	ebx,#src1		; get pointer to color 1
	mov	ebx,[ebx]		; get rgb values for color 1
	mov	esi,#src2		; get pointer to color 2
	mov	edi,#dest		; get pointer to output
	mov	dh,#percent		; dl = percent
	mov	cx,6464h		; ch = 100, cl = 100
	sub	ch,dh			; ch = vpercent (100-percent)

	mov	al,bl			; get src1->r
	mul	ch			; ax = src1->r * vpercent
	mov	ebp,eax 		; remember it
	mov	al,[esi]		; get src2->r
	mul	dh			; ax = src2->r * percent
	lea	eax,[eax+ebp+50]	; sum 'em up
	div	cl			; divide by 100
	mov	[edi],al		; save the result

	mov	al,bh			; get src1->g
	mul	ch			; ax = src1->g * vpercent
	mov	ebp,eax 		; remember it
	mov	al,[esi+1]		; get src2->g
	mul	dh			; ax = src2->g * percent
	lea	eax,[eax+ebp+50]	; sum 'em up
	div	cl			; divide by 100
	mov	[edi+1],al		; save the result

	shld	eax,ebx,16		; get src1->b (tricky but it works)
	mul	ch			; ax = src1->b * vpercent
	mov	ebp,eax 		; remember it
	mov	al,[esi+2]		; get src2->b
	mul	dh			; ax = src2->b * percent
	lea	eax,[eax+ebp+50]	; sum 'em up
	div	cl			; divide by 100
	mov	[edi+2],al		; save the result

	Restore esi,ebx,edi,ebp
	Exit

true_blend endp

_TEXT	ends
	end


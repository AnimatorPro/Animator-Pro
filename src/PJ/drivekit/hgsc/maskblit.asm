;*****************************************************************************
;* maskblit.asm - Generic assembler routines to handle maskblit logic.
;*   The routines in this module (mask1line and mask2line ) can be used by any
;*   driver that wants to implement the maskxblit functions in terms of a loop
;*   that does get_hseg/maskxline/put_hseg.  This is pretty much what the PJ
;*   generic library does, but a driver can do it faster because several
;*   levels of indirect function calls will be eliminated.
;*
;*   See the RASTER.C module in the HGSC driver for an example of using this.
;*****************************************************************************

CGROUP	group	_CODE
_CODE	segment public para use32 'CODE'
	assume	cs:CGROUP

	public	mask1line
	public	mask2line

mlparms  struc
mbytes	 dd	?
lbuf	 dd	?
count	 dd	?
bit1	 db	?
oncolor  db	?
offcolor db	?
mlparms  ends

;----------------------------------------------------------------------------
; void mask1line(struct m1lparms *pparms);
;  Expand a bitmask into a line of pixels in a buffer, where 1's in the
;  bitmask result in 'oncolor' being placed into the buffer, and 0's in the
;  bitmask leave the corresponding pixel unchanged in the buffer.
;----------------------------------------------------------------------------

mask1line proc near

	mov	eax,[esp+4]		; load parmptr
	push	esi
	push	edi

	mov	esi,[eax].mbytes	; load pointer to bitmask array
	mov	edi,[eax].lbuf		; load pointer to pixel buffer
	mov	ecx,[eax].count 	; load count of bits to process
	mov	dl,[eax].bit1		; load starting bit-test mask value
	mov	dh,[eax].oncolor	; load color of 'on' pixels
	lodsb				; get first byte from bitmask array
mask1loop:
	test	al,dl			; test current bit in bitmask.
	jz	short m1notset		; if zero, leave buffer unchanged,
	mov	[edi],dh		; else put oncolor value into buffer
m1notset:
	inc	edi			; increment pixel buffer pointer.
	shr	dl,1			; shift to next test bit. if shift
	loopnz	mask1loop		; result & counter both non-zero loop,
	jecxz	short m1done		; else if counter is zero, done, else
	mov	dl,80h			; load a fresh bit-test mask, and get
	lodsb				; the next byte from the mask array,
	jmp	short mask1loop 	; then loop to do some more.
m1done:
	pop	edi
	pop	esi
	ret

mask1line endp

;----------------------------------------------------------------------------
; void mask2line(struct m1lparms *pparms);
;  Expand a bitmask into a line of pixels in a buffer, where 1's in the
;  bitmask result in 'oncolor' being placed into the buffer, and 0's in the
;  bitmask results in 'offcolor' being placed into the buffer.
;----------------------------------------------------------------------------

mask2line proc near

	mov	eax,[esp+4]		; load parmptr
	push	esi
	push	edi

	mov	esi,[eax].mbytes	; load pointer to bitmask array
	mov	edi,[eax].lbuf		; load pointer to pixel buffer
	mov	ecx,[eax].count 	; load count of bits to process
	mov	dl,[eax].bit1		; load starting bit-test mask value
	mov	dh,[eax].oncolor	; load color of 'on' pixels
	mov	ah,[eax].offcolor	; load color of 'off' pixels
	lodsb				; get first byte from bitmask array
mask2loop:
	test	al,dl			; test current bit in bitmask.
	jz	short m2notset		; if zero, leave buffer unchanged,
	mov	[edi],dh		; else put oncolor value into buffer
	jmp	short m2isset		; and skip to increment of pointer.
m2notset:
	mov	[edi],ah		; put offcolor value into buffer
m2isset:
	inc	edi			; increment pixel buffer pointer.
	shr	dl,1			; shift to next test bit. if shift
	loopnz	mask2loop		; result & counter both non-zero loop,
	jecxz	short m2done		; else if counter is zero, done, else
	mov	dl,80h			; load a fresh bit-test mask, and get
	lodsb				; the next byte from the mask array,
	jmp	short mask2loop 	; then loop to do some more.
m2done:

	pop	edi
	pop	esi
	ret

mask2line endp

_CODE	ends
	end

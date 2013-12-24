;*****************************************************************************
;* CONVCLK.ASM - Code to convert clock values between milliseconds and jiffies
;*****************************************************************************

	include stdmacro.i

_DATA	segment

clockscale  dd	(256*1000)/70
SHIFTSCALE  equ 8

_DATA	ends

_TEXT	segment

	extrn	pj_clock_1000:near
	public	pj_clock_jiffies
	public	_pj_clock_jiffies
	public	pj_clock_ms2jiffies
	public	_pj_clock_ms2jiffies
	public	pj_clock_jiffies2ms
	public	_pj_clock_jiffies2ms


;*****************************************************************************
;* ULONG pj_clock_jiffies(void)
;*****************************************************************************

pj_clock_jiffies  proc near
_pj_clock_jiffies proc near

	call	pj_clock_1000
	cdq
	shld	edx,eax,SHIFTSCALE
	shl	eax,SHIFTSCALE
	div	clockscale
	ret

_pj_clock_jiffies endp
pj_clock_jiffies  endp

;*****************************************************************************
;* ULONG pj_clock_ms2jiffies(ULONG ms)
;*****************************************************************************

pj_clock_ms2jiffies  proc near
_pj_clock_ms2jiffies proc near

	Entry
	Args	#ms
	mov	eax,#ms
	cdq
	shld	edx,eax,SHIFTSCALE
	shl	eax,SHIFTSCALE
	div	clockscale
	Exit

_pj_clock_ms2jiffies endp
pj_clock_ms2jiffies  endp

;*****************************************************************************
;* ULONG pj_clock_jiffies2ms(ULONG jiffies)
;*****************************************************************************

pj_clock_jiffies2ms  proc near
_pj_clock_jiffies2ms proc near

	Entry
	Args	#jiffies
	mov	eax,#jiffies
	mul	clockscale
	shrd	eax,edx,SHIFTSCALE
	Exit

_pj_clock_jiffies2ms endp
pj_clock_jiffies2ms  endp


_TEXT	ends
	end

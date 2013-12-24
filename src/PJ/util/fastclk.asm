;*****************************************************************************
;* FASTCLK.ASM - Code to access the 1.19mhz clock.
;*****************************************************************************

CGROUP	group	code
DGROUP	group	data

data	segment public dword use32 'DATA'

clockdata  struc
lastclock  dd	0			; the last value we returned
lclockroll dd	0			; adjustment for midnight rollovers
hclockroll dd	0			; adjustment for midnight rollovers
mhz_to_ms  dd	1193180/1000		; divisor to turn clock into millisecs
initdone   db	0			; flag: has init been done?
clockdata  ends

clockctl   clockdata <> 		; define an instance of clockdata

data	ends

code	segment public dword use32 'CODE'
	assume	cs:CGROUP,ds:DGROUP

CMODE	equ	043h
CDATA	equ	040h
CTIME	equ	46ch

TICKSPERDAY equ 1573040 		; ticks in a day according to BIOS.
	       ;1572480 		; this is what I calc'd; it don't work.

	public	pj_clock_1000
	public	pj_clock_init
	public	pj_clock_cleanup

	extrn	pj_set_gs:near

;*****************************************************************************
;* unsigned long pj_clock_1000(void)
;*
;*  return the number of elapsed milliseconds since midnight of the day the
;*  program was started.  useful for delta timings.  returned value is
;*  accurate to within about .5 millisecond.  (that is, if you're trying to
;*  wait 1ms by calling this routine in a loop, you may find that 1.4ms
;*  have elapsed before you get control back.  on the other hand, when waiting
;*  for longer time periods, the error as a percentage of the wait period
;*  becomes small enough to be ignored.)  this clock rolls over after 49.7
;*  days of continuous running.
;*****************************************************************************

	align 4
pj_clock_1000 proc near

	lea	ecx,clockctl		; load pointer to clock control struct.
#try_again:
	cli				; block interupts; this prevents 99% of
	mov	al,0			; the glitches that occur when we try
	out	CMODE,al		; to assemble the interupt-driven BIOS
	mov	al,1			; portion of the time and the chip
	out	CMODE,al		; counter portion into a 48-bit value.
	in	al,CDATA		; we get the 16 bits of chip counter
	ror	eax,8			; (low then high order parts, but
	in	al,CDATA		; 'IN AX' doesn't work).  next get the
	mov	edx,gs:[CTIME]		; BIOS timeofday counter from low
	sti				; memory.  when we have both, we can
	ror	eax,8			; re-enable interupts.
	shrd	eax,edx,16		; at this point, chip time is in high
	shr	edx,16			; order eax and bios time is in edx;
	neg	ax			; shift the qword down 16 bits so the
	dec	ax			; 48-bit value is aligned in edx:eax.

	add	eax,[ecx].lclockroll	; add in midnight rollover low order.
	adc	edx,[ecx].hclockroll	; add in midnight rollover high order.

;-----------------------------------------------------------------------------
; at this point, edx:eax is the number of 1.19318mhz ticks since midnight of
; the day the program was started.
;-----------------------------------------------------------------------------

	div	[ecx].mhz_to_ms 	; turn 1.19mhz counter into millisecs.
	cmp	eax,[ecx].lastclock	; compare milleseconds value to last
	jl	short #handle_spike	; value we generated.  if lower, go
	mov	[ecx].lastclock,eax	; check for midnight rollover or
	ret				; interupt latency problem, else return.

;-----------------------------------------------------------------------------
; here we handle a backwards 'spike' of the clock.  these spikes happen for
; two reasons:	when the bios timer rolls from 23:59:59 to zero at midnight,
; and when the chip counter has just rolled over and the bios part of the
; timer hasn't been updated yet because of interupt latency on the (generally
; heavily-hooked) INT8 vector. when the interupt latency problem occurs,
; it generates a backwards spike of 55ms in the return value (one 18.2th
; of a second), but for safety's sake, we consider anything up to 1000ms
; (18 interupts) to be this sort of spike.  when this occurs, we just loop
; back to the get-the-48-bit-value part of the routine, above, and get a
; new clock value.  hopefully the interupt processing will have caught up
; by that point, and the bios and chip timers will appear to be in sync again.
; (in a worst case, we may end up going through here twice, but I've yet to
; see it happen).  any spike value bigger than 1000ms we consider to be a
; midnight rollover, and we increase our rollover add-in value by the number
; of bios counter ticks in a day, then loop back to recalc the time.
; (note: resist the temptation to change the midnight rollover detection
; logic here.  while it seems 'natural' to check the bios clock for a very
; low value instead of checking to see if we might have latency, it doesn't
; work well.  for example, suppose you try to check the bios timer for a
; value indicating that it is between 00:00:00 and 00:05:00 as the way of
; knowing whether a midnight rollover happened...if we get an interupt
; latency hit during that period, it would be processed as a false rollover.)
; (also note: this whole interupt latency thing is a very rare situation. I
; can only get it to happen when I stack a bunch of pj_clock_1000() calls
; together with no intervening code.  (ie, I'm allowing maybe 20 microseconds
; to elapse between calls to pj_clock_1000()).	'normal' usage of the clock
; routine where at least some processing occurs between calls will never
; encounter the latency glitches.)
;-----------------------------------------------------------------------------

	align 4
#handle_spike:

	add	eax,1000		; these lines deal with the interupt
	cmp	eax,[ecx].lastclock	; latency problem that happens when
	jae	short #try_again	; we grab a just-rolled chip clock.

	xor	edx,edx 		; these lines deal with midnight
	mov	eax,TICKSPERDAY 	; rollover.  we get the TICKSPERDAY
	shld	edx,eax,16		; constant, which is an adjustment to
	shl	eax,16			; the bios part of the timer value, so
	add	[ecx].lclockroll,eax	; we have to upshift it 16 bits to
	adc	[ecx].hclockroll,edx	; align it properly, then we remember
	jmp	#try_again		; the adjustment and recalc the time.

pj_clock_1000 endp

;*****************************************************************************
;* Boolean pj_clock_init(void)
;*
;*   program the hardware timer to run in the mode we need.  this routine
;*   can be called any number of times without affecting delta timings
;*   across calls to the init routine.	(eg, you can grab a timer value,
;*   shell out to another program, on return call this to re-init the clock
;*   in case the program reprogrammed the hardware, then grab a timer value
;*   again and use it in calc'ing how long the shelled program was running.)
;*
;*   the return value indicates whether the clock was already init'd before
;*   this routine was called.  this is used by the flilib code to implement
;*   a 'conditional cleanup', wherein a hidden internal routine can init the
;*   clock, then do a cleanup only if the clock hadn't been init'd already
;*   by the client code.
;*****************************************************************************

pj_clock_init proc near

	call	pj_set_gs		; go load GS segreg

	cli
	mov	al,00110100b		; put clock into linear count instead
	out	CMODE,al		; of divide-by-two mode.
	xor	al,al
	out	CDATA,al
	out	CDATA,al
	sti

	movzx	eax,clockctl.initdone	; set return value (was clock already
	mov	clockctl.initdone,1	; init'd).  set initdone flag to TRUE.

	ret

pj_clock_init endp

;*****************************************************************************
;* void pj_clock_cleanup(void)
;*
;*   program the hardware to run in normal BIOS mode (mode 3).	also, zero
;*   out our internal rollover counters and such.  the only reason for the
;*   zero-out is to provide an application that wants to run more than 49.7
;*   days with a way of restarting the clock (ie, midnight rollover biases)).
;*
;*   we also zero out the initdone flag, to indicate that the clock is now
;*   'not initialized', so that repeated init/cleanup sequences will return
;*   the right status from the init calls.
;*****************************************************************************

pj_clock_cleanup proc near

	cli
	mov	al,00110110b		;put it back to divide by 2
	out	CMODE,al
	xor	al,al
	out	CDATA,al
	out	CDATA,al
	sti

	lea	ecx,clockctl
	xor	eax,eax
	mov	[ecx].lastclock,eax
	mov	[ecx].lclockroll,eax
	mov	[ecx].hclockroll,eax
	mov	[ecx].initdone,al

	ret

pj_clock_cleanup endp


code	ends
	end

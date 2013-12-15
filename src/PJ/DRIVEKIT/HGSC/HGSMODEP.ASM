;*****************************************************************************
;* HGSMODEP.ASM - Protected-mode code to set up HGSC SVGA modes.
;*****************************************************************************

	include hgsc.inc
	include macro.inc

;-----------------------------------------------------------------------------
; Tables contains HGSC mode data for each of the modes...
;-----------------------------------------------------------------------------

_DATA	segment para public use32 'DATA'

m512x480:
					;HGSC Integrated CRT registers...
	dw	08			;HESYNC - Horizontal parameters
	dw	12			;HEBLNK
	dw	76			;HSBLNK
	dw	78			;HTOTAL

	dw	01			;VESYNC - Vertical parameters
	dw	35			;VEBLNK
	dw	515			;VSBLNK
	dw	527			;VTOTAL

	dw	0f040h			;DPYCTL - Control regs
	dw	0fffch			;DPSTRT
	dw	0000			;DPYTAP - horizontal start address

					; HGSC DAC & Config registers...
	dw	01h			; clk_sel Hercules I/O registers
	dw	01h OR 0Ch		; pixel size | sync polarity
	dw	017H			; dac_cmd, truecolor

	dw	512			;line size
	dw	480			;number of lines
	dw	32			;pixel size
	dw	512*4*8 		;pitch
	dw	14			;pitch shift-multiplier (eg, 2**n == pitch)

; - 25.175 MHz PARAMETERS -

m640x480x8:
	dw	11			;HESYNC
	dw	16			;HEBLNK
	dw	96			;HSBLNK
	dw	99			;HTOTAL

	dw	01			;VESYNC
	dw	33			;VEBLNK
	dw	513			;VSBLNK
	dw	524			;VTOTAL

	dw	0f020h			;DPYCTL
	dw	0fffch			;DPSTRT
	dw	0000			;DPYTAP

	dw	06h			;clk sel
	dw	03h OR 0Ch		;pixel size | sync polarity
	dw	05BH			;dac cmd, 8 bit pixel from green w/24bit LUT

	dw	640			;line size
	dw	480			;number of lines
	dw	8			;pixel size
	dw	1024*8			;pitch
	dw	13			;pitch shift-multiplier

; - 50.350 MHz PARAMETERS -

m640x480x16:
	dw	23			;HESYNC
	dw	34			;HEBLNK
	dw	194			;HSBLNK
	dw	199			;HTOTAL

	dw	01			;VESYNC
	dw	33			;VEBLNK
	dw	513			;VSBLNK
	dw	525			;VTOTAL

	dw	0f040h			;DPYCTL
	dw	0fffch			;DPSTRT
	dw	0000			;DPYTAP

	dw	07h			;clk_sel
	dw	02h OR 0Ch		;pxsize | syncp
	dw	0CFh			;dac_cmd, truecolor

	dw	640			;line size
	dw	480			;number of lines
	dw	16			;pixel size
	dw	1024*2*8		;pitch
	dw	14			;pitch shift-multiplier


; - 36 MHz PARAMETERS -

m800x600:
	dw	08			;HESYNC
	dw	24			;HEBLNK
	dw	124			;HSBLNK
	dw	127			;HTOTAL

	dw	01			;VESYNC
	dw	23			;VEBLNK
	dw	623			;VSBLNK
	dw	624			;VTOTAL

	dw	0f020h			;DPYCTL
	dw	0fffch			;DPSTRT
	dw	0000			;DPYTAP

	dw	03h			;clk_sel
	dw	03h OR 0Ch		;pxsize | syncp
	dw	05Bh			;dac cmd, 8 bit pixel from green w/24bit LUT

	dw	800			;line size
	dw	600			;number of lines
	dw	8			;pixel size
	dw	1024*8			;pitch
	dw	13			;pitch shift-multiplier


; - 44.9 MHz PARAMETERS -

m1024x768i:
	dw	21			;HESYNC
	dw	28			;HEBLNK
	dw	156			;HSBLNK
	dw	157			;HTOTAL

	dw	03			;VESYNC
	dw	22			;VEBLNK
	dw	406			;VSBLNK
	dw	408			;VTOTAL

	dw	0b040h			;DPYCTL
	dw	0fffch			;DPSTRT
	dw	0000			;DPYTAP

	dw	04h			;clk_sel
	dw	03h OR 00h		;pxsize | syncp
	dw	05Bh			;dac cmd, 8 bit pixel from green w/24bit LUT

	dw	1024			;line size
	dw	768			;number of lines
	dw	8			;pixel size
	dw	1024*8			;pitch
	dw	13			;pitch shift-multiplier

; - 64 MHz PARAMETERS -

m1024x768n:
	dw	07			;HESYNC
	dw	32			;HEBLNK
	dw	160			;HSBLNK
	dw	164			;HTOTAL

	dw	03			;VESYNC
	dw	30			;VEBLNK
	dw	798			;VSBLNK
	dw	799			;VTOTAL

	dw	0f020h			;DPYCTL
	dw	0fffch			;DPSTRT
	dw	0000			;DPYTAP

	dw	05h			;clk_sel
	dw	03h OR 0Ch		;pxsize | syncp
	dw	05Bh			;dac cmd, 8 bit pixel from green w/24bit LUT

	dw	1024			;line size
	dw	768			;number of lines
	dw	8			;pixel size
	dw	1024*8			;pitch
	dw	13			;pitch shift-multiplier

;-----------------------------------------------------------------------------
; Table of pointers to the mode data tables...
;-----------------------------------------------------------------------------

mode_table:
	dd	m640x480x8
	dd	m640x480x16
	dd	m512x480
	dd	m800x600
	dd	m1024x768i
	dd	m1024x768n

;-----------------------------------------------------------------------------
; special much-abbreviated mode data for standard VGA mode...
;-----------------------------------------------------------------------------

v_clksel  dw	02h			; normal VGA clock select value
v_szsyncp dw	0Ch			; normal VGA pixel size & syncp
v_daccmd  dw	04Bh			; normal VGA DAC, 8 bit pix w/18 bit LUT


_DATA	ends

;*****************************************************************************
;* Code...
;*****************************************************************************

_CODE	segment para public use32 'CODE'

	public	hgs_setmode
	public	hgs_setpalette
	public	hgs_wait_vsync

;-----------------------------------------------------------------------------
; bus12_on - Enable 16-bit bus transfers in VGA memory page (0x000A0000).
;-----------------------------------------------------------------------------

bus16_on	 proc			near

	push	edx
	mov	edx,46E8h
	mov	eax,0
	out	dx,ax			 ; Set up the 16 bit CPU mode
	pop	edx
	ret

bus16_on	 endp


;-----------------------------------------------------------------------------
; bus16_off - Disable 16-bit transfers (required for normal VGA operations).
;-----------------------------------------------------------------------------

bus16_off  proc  near

	push	edx
	mov	edx,46E8h
	mov	eax,0Eh
	out	dx,ax			 ; Clear the 16 bit CPU mode
	pop	edx
	ret

bus16_off  endp

;---------------------------------------------------------------------------
; void hgs_setmode(int mode, HWModedata **pmd);
;
;  Set up a given HGSC video mode.  A pointer to the mode data table is
;  returned via the 'pmd' parameter, if pmd is not NULL.  This routine can be
;  called regardless of the current state of the HGSC board.  If it is called
;  to set normal VGA mode (TM_VGA), it will exit with the 16-bit bus disabled
;  and normal VGA on the screen.  If called to setup any other mode, it will
;  exit with the 16-bit bus mode enabled, which disables normal VGA
;  operations, including the VGA BIOS and access to VGA regs.
;
;---------------------------------------------------------------------------

hgs_setmode proc near

	Entry
	Save	ebx,esi,edi,es

	call	bus16_off		; must be off for mode changes to work
	call	hgs_wait_vsync		; screen change looks cleaner during vblank

	mov	ax,SS_DOSMEM
	mov	es,ax

	mov	eax,[ebp+8]		; load requested mode
	cmp	eax,-1			; normal VGA mode ?
	jne	native_modes		; nope, go do SVGA setup
	call	setvga			; yep, go setup normal VGA
	jmp	short exit_sm		; then exit
native_modes:
	lea	esi,mode_table		; point to SVGA pointer table
	mov	esi,[eax*4+esi] 	; point to modedata entry
	mov	ecx,[ebp+12]		; get pointer to return val
	cmp	ecx,0			; is pointer NULL?
	je	no_retval		; yep, don't try to use it
	mov	[ecx],esi		; return modedata pointer
no_retval:
	call	set_mode		; go setup SVGA/truecolor mode
	call	bus16_on		; turn on fast bus for SVGA
exit_sm:
	Restore ebx,esi,edi,es
	Exit

hgs_setmode endp

;---------------------------------------------------------------------------
; set_mode - Internal service routine to set up SVGA/truecolor modes.
;
;  Entry:
;	esi = pointer to video parameter table
;	      all registers preserved by caller, any can be modified by this
;	      routine.
;  Exit:
;	all registers may be trashed
;
;  Note: bus16 will be off during this routine, don't use HDATA16 here.
;---------------------------------------------------------------------------

set_mode proc	near

	mov	wpes:[HCTRL],0D800h	; halt 34010, incr, incw

	mov	wpes:[HADDL],HESYNC	; point 34010 to HESYNC register
	mov	wpes:[HADDH],HCRTSEG	; in CRT controller memory segment

	mov	edi,HDATA		; host data reg address
	mov	ecx,10			; number of consecutive registers
	rep movsw			; copy the timing values into the 34010

	mov	wpes:[HCTRL],0C00h	; halt 34010, no incr, no incw

	mov	wpes:[HADDL],DPYTAP	; point 34010 to DPYTAP register
	mov	wpes:[HADDH],HCRTSEG	; in CRT controller memory segment
	movsw				; copy the DPYTAP value

	mov	wpes:[HADDL],CONFG1W	; point 34010 at config 1 register
	mov	wpes:[HADDH],HREGSEG	; in register memory segment
	lodsw				; get clk_sel from mode table
	mov	ecx,eax 		; save for later
	or	ax,00001000b		; include rs2 = 1 in clk_sel
	stosw				; write it to the config 1 register

	mov	wpes:[HADDL],CONFG2W	; point 34010 at config 2 register
	mov	wpes:[HADDH],HREGSEG	; in register memory segment
	movsw				; copy pxsize|syncp value

	mov	wpes:[HADDL],DACCMD	; point 34010 at dac command reg
	mov	wpes:[HADDH],HREGSEG	; in register memory segment
	movsw				; copy dac command value

	mov	wpes:[HADDL],CONFG1W	; point 34010 to config 1 register
	mov	wpes:[HADDH],HREGSEG	; in register memory segment
	mov	eax,ecx 		; get clock sel value back
	stosw				; write to the clock_sel/rs2 port

	mov	wpes:[HADDL],0		; reset host addresss pointers
	mov	wpes:[HADDH],0		; (not sure this is needed)

	ret				; done.

set_mode	endp

;-----------------------------------------------------------------------------
; setvga - Internal service routine to set normal VGA mode.
;
;  Entry:
;	all registers preserved by caller, any can be modified by this routine.
;  Exit:
;	all registers may be trashed
;
;  Note: bus16 will be off during this routine, don't use HDATA16 here.
;-----------------------------------------------------------------------------

setvga	proc	 near

	mov	wpes:[HCTRL],0C000h	; halt 34010, no incr, no incw

	mov	wpes:[HADDL],CONFG2W
	mov	wpes:[HADDH],HREGSEG
	mov	ax,v_szsyncp
	mov	es:[HDATA],ax

	mov	wpes:[HADDL],CONFG1W
	mov	wpes:[HADDH],HREGSEG
	mov	ax,v_clksel
	or	ax,00001000b
	mov	es:[HDATA],ax		 ; vga clksel w/rs2 bit set

	mov	dx,03c6h
	mov	ax,v_daccmd
	out	dx,al

	mov	wpes:[HADDL],CONFG1W
	mov	wpes:[HADDH],HREGSEG
	mov	ax,v_clksel
	mov	es:[HDATA],ax		; reset rs2 bit

	mov	wpes:[HADDL],0
	mov	wpes:[HADDH],0

	ret

setvga	endp

;---------------------------------------------------------------------------
; hgs_setpalette - Set a block of palette entries.
;
;  Entry:
;    setpalette(void *data, int start, int count, PALET *palet);
;  Exit:
;    Per Watcom 3s conventions.
;
;  Note:
;    There is a bug in the HGSC that complicates setting the pallete from
;    SVGA mode.  When the I/O transfer address is set to point to the DAC
;    pallete register, the HGSC does a pre-fetch of the value in the reg.
;    Unfortunately, the prefetch increments the DAC palette index, and data
;    transfers that follow are misaligned by one byte.	(IE, the first RED
;    value is prefetched, then the first red value written goes into the green
;    slot, and so on.)	The Hercules-recommended workaround for this is to set
;    the palette index register to one less than the entry you really want to
;    set.  This cause a prefetch.  Then two dummy reads are done, leaving the
;    DAC palette index pointing to the first register to be set.  More
;    details can be found in the file PALET.DOC, available on the Hercules
;    support BBS.
;---------------------------------------------------------------------------

hgs_setpalette	 proc	   near

start	equ	[ebp+12]
count	equ	[ebp+16]
palette equ	[ebp+20]

	Entry
	Save	esi,es

	mov	ax,SS_DOSMEM
	mov	es,ax

	mov	wpes:[HCTRL],0C000h	; halt 34010, no incr, no incw

	mov	wpes:[HADDL],LUTIDXW	;
	mov	wpes:[HADDH],HREGSEG
	mov	eax,start
	dec	al			; point to the reg before the one we
	mov	es:[HDATA16],ax 	; want, per Herc's palet.doc workaround

	mov	wpes:[HADDL],LUTDAT
	mov	wpes:[HADDH],HREGSEG
	mov	ax,es:[HDATA16] 	; dummy read, per Herc's palet.doc
	mov	ax,es:[HDATA16] 	; another dummy read

	mov	esi,palette		; load pointer to rgb values
	mov	ecx,count		; load count of entries to be set
dacloop:
	lodsb
	mov	es:[HDATA16],ax
	lodsb
	mov	es:[HDATA16],ax
	lodsb
	mov	es:[HDATA16],ax
	loop	dacloop

	Restore esi,es
	Exit

hgs_setpalette	endp

;---------------------------------------------------------------------------
; void hgs_wait_vsync(void)
;
;  Wait until the monitor enters vblank, or return right away if we are
;  currently in vblank and at least a few scanlines of blanking time remain
;  before the display is enabled again.
;
;  Note: This routine must not use HDATA16 addressing -- it may be
;	 called when bus16 is off!
;---------------------------------------------------------------------------

hgs_wait_vsync proc near

	Entry
	Save	ebx,ds

	mov	ax,SS_DOSMEM
	mov	ds,ax

	mov	wpds:[HCTRL],0C000h	; halt 34010, no incr, no incw

	mov	wpds:[HADDL],CONFG2R	; check the config2 value to see if
	mov	wpds:[HADDH],HREGSEG	; normal VGA is in control of the card.
	mov	ax,wpds:[HDATA] 	; if it is we punt, because if the
	test	eax,3			; card has been in normal VGA since
	jz	short in_vblank 	; powerup the vblank wait loops forever.

	mov	wpds:[HADDL],VSBLNK	; fetch the start-of-vblank
	mov	wpds:[HADDH],HCRTSEG	; value, for later comparison
	movzx	ebx,wpds:[HDATA]	; against vcount.

	mov	wpds:[HADDL],VEBLNK	; fetch end-of-vblank, but
	mov	wpds:[HADDH],HCRTSEG	; divide it by 2; we wanna
	movzx	ecx,wpds:[HDATA]	; make sure vblank will last
	shr	ecx,1			; a few lines after we return.
waitmore:
	mov	wpds:[HADDL],VCOUNT
	mov	wpds:[HADDH],HCRTSEG
	movzx	eax,wpds:[HDATA]	; fetch end-of-vblank value

	cmp	eax,ebx 		; if count > start
	ja	in_vblank		; we are in vblank
	cmp	eax,ecx 		; if count >= (end/2)
	jae	waitmore		; we are not, go wait some more
in_vblank:
	Restore ebx,ds
	Exit

hgs_wait_vsync endp

_CODE	ends
	end


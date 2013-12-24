;*****************************************************************************
;* Include file for the HGSC display driver assembler modules...
;*****************************************************************************

CGROUP	group	_CODE
DGROUP	group	_DATA

_DATA	segment public para use32 'DATA'
_DATA	ends
_CODE	segment public para use32 'CODE'
_CODE	ends
		assume	cs:CGROUP,ds:DGROUP

SS_DOSMEM equ 034H

;*****************************************************************************
;* Stuff for the HGSC and TMS34010...
;*****************************************************************************

modedata  struc

sHESYNC   dw		?
sHEBLNK   dw		?
sHSBLNK   dw		?
sHTOTAL   dw		?

sVESYNC   dw		?
sVEBLNK   dw		?
sVSBLNK   dw		?
sVTOTAL   dw		?

sDPYCTL   dw		?
sDPSTRT   dw		?
sDPYTAP   dw		?

sCLKSEL   dw		?
sPIXSYNC  dw		?
sDACCMD   dw		?

sXSIZE	  dw		?
sYSIZE	  dw		?
sPSIZE	  dw		?
sPITCH	  dw		?
sYSHFMUL  dw		?		; (Ian) added - shift factor for y-addr multiplier

modedata  ends

;-----------------------------------------------------------------------------
; HGSC Host port addresses (protected-mode 32 bit offsets from abs addr 0)
;
;  Note a bit of trickiness here... The HDATA16 constant, which addresses
;  into the VGA segment for fast 16-bit access to the HGSC data port, can
;  point to any location within the range 0x000A0000 to 0x000AFFFF.  We set
;  it to the middle of the range.  This allows routines that do 'rep stos'
;  and 'rep lods' instructions to disregard the state of the 80386 direction
;  flag.  Only routines which use 'movs' instructions need to explicitly set
;  the flag, since they work within main memory, which is not blind to
;  direction like the HGSC data register interface is.
;-----------------------------------------------------------------------------

HADDH	  equ		000C7F00h		; Address high word
HADDL	  equ		000C7E00h		; Address low word
HCTRL	  equ		000C7D00h		; Host control register
HDATA	  equ		000C7000h		; Host data register-slow
HDATA16   equ		000A8000h		; Host data register for 16-bit bus mode

;-----------------------------------------------------------------------------
; HGSC CRT controller registers
;  all of these are offset from HCRTDSEG in the hgsc addr space
;-----------------------------------------------------------------------------

HCRTSEG   equ		0C000h			; Host crt ctlr seg, in host addr space
HTOTAL	  equ		0030h			; HTOTAL register
HSBLNK	  equ		0020h			; HORIZONTAL START BLANK REGISTER
HEBLNK	  equ		0010h			; HORIZONTAL END BLANK REGISTER
HESYNC	  equ		0000h			; HORIZONTAL END SYNC REGISTER

VTOTAL	  equ		0070h			; HTOTAL register
VSBLNK	  equ		0060h			; HORIZONTAL START BLANK REGISTER
VEBLNK	  equ		0050h			; HORIZONTAL END BLANK REGISTER
VESYNC	  equ		0040h			; HORIZONTAL END SYNC REGISTER
VCOUNT	  equ		01D0h			; Vertical line counter (I hope...)

DPYCTL	  equ		0080h			; Display Control Register
DPYTAP	  equ		01B0h			; Display hscroll tap register

;-----------------------------------------------------------------------------
; HGSC DAC and configuration registers...
;	all of these are offset from HREGSEG in the hgsc addr space
;-----------------------------------------------------------------------------

HREGSEG   equ		0600h			; Host register seg, in host addr space
LUTIDXW   equ		0000h			; Lookup table write start index
LUTDAT	  equ		0010h			; Lookup table data
DACCMD	  equ		0020h			; DAC command register
LUTIDXR   equ		0030h			; Lookup table read start index
CONFG1R   equ		01C0h			; Configuration register 1 read
CONFG1W   equ		00C0h			; Configuration register 1 write
CONFG2R   equ		01E0h			; Configuration register 2 read
CONFG2W   equ		00E0h			; Configuration register 2 write

;-----------------------------------------------------------------------------
; Indicies into the modedata pointer table...
;-----------------------------------------------------------------------------

TM_VGA			equ   -1
TM_640x480x8	equ   0
TM_640x480x16	equ   1
TM_512x480		equ   2
TM_800x600		equ   3
TM_1024x768i	equ   4
TM_1024x768n	equ   5

;*****************************************************************************
;* Stuff specific to our driver...
;*****************************************************************************

YSHFTMUL		equ   13		; Y << 13 gets to proper line in HGSC memory
YNEXTLINE		equ   8192		; Add this to current offset to get to next line

;-----------------------------------------------------------------------------
; Our Hrast structure...
;-----------------------------------------------------------------------------

hrast	struc
rast_type		dw ?
rast_pdepth 	dw ?
rast_lib		dd ?
rast_aspect_dx	dw ?
rast_aspect_dy	dw ?
rast_reserved	dd ?
rast_width		dw ?
rast_height 	dw ?
rast_x			dw ?
rast_y			dw ?
rast_grclib 	dd ?
hrast ends

;-----------------------------------------------------------------------------
; some functions used by higher-level routines...
;-----------------------------------------------------------------------------

		extrn	_hgs_put_ohseg:near
		extrn	_hgs_put_ehseg:near
		extrn	_hgs_get_ohseg:near
		extrn	_hgs_get_ehseg:near

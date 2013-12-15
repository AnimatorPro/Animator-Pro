
	name	HARDWARE
	subttl	hardware.asm -- most commonly used hardware routines



;******************************************************************************
;									      *
;		   Copyright (C) 1991 by Autodesk, Inc. 		      *
;									      *
;	Permission to use, copy, modify, and distribute this software and     *
;	its documentation for the purpose of creating applications for	      *
;	AutoCAD, is hereby granted in accordance with the terms of the	      *
;	License Agreement accompanying this product.			      *
;									      *
;	Autodesk makes no warrantees, express or implied, as to the	      *
;	correctness of this code or any derivative works which incorporate    *
;	it.  Autodesk provides the code on an ''as-is'' basis and             *
;	explicitly disclaims any liability, express or implied, for	      *
;	errors, omissions, and other problems in the code, including	      *
;	consequential and incidental damages.				      *
;									      *
;	Use, duplication, or disclosure by the U.S.  Government is	      *
;	subject to restrictions set forth in FAR 52.227-19 (Commercial	      *
;	Computer Software - Restricted Rights) and DFAR 252.227-7013 (c)      *
;	(1) (ii) (Rights in Technical Data and Computer Software, as	      *
;	applicable.							      *
;									      *
;******************************************************************************

;   4/18/91  - jdb - put into ADI stream (created from \vesa driver)


;*****************************************************************************
;
;   INCLUDES
;
;*****************************************************************************

	include compaq.i

;*****************************************************************************
;
;   DEFINES
;
;*****************************************************************************

VOFFSET macro	X, Y
	mov	ebx, pitch
	imul	ebx, Y
	add	ebx, X
	endm

PACKED_PIXEL	equ	4

VIDEO_INT	equ	10h	    ; DOS BIOS video interrupt

VESA_GET_INFO	equ	4f00h	    ; get VESA hardware information
VESA_MODE_INFO	equ	4f01h	    ; get video mode information
VESA_SET_MODE	equ	4f02h	    ; set video mode
VESA_GET_MODE	equ	4f03h	    ; get video mode
VESA_WIN_FUNC	equ	4f05h	    ; window functions

CAN_READ	equ	3
CAN_WRITE	equ	5
CAN_READ_WRITE	equ	7


SUCCESSFUL	equ	004fh	    ; VESA function supported (and successful)

SUCCESS 	equ	0
FAILURE 	equ	(-1)

ATTR_MODE_SUPPORTED equ     1
ATTR_EXTENDED_INFO  equ     2
ATTR_IS_GRAPHICS    equ     10h

;*****************************************************************************

_stack	segment byte stack use32 'stack'
_stack	ends

;*****************************************************************************

_data	segment

	assume	ds:DGROUP




	public	VESAInfoBlock
VESAInfoBlock	infostruc   <>

	public	VESAModeBlock
VESAModeBlock	modestruc   <>	    ; VESA Mode Information Block

	public	VESAModes
VESAModes	dw  MAXVESAMODES dup ( 0 ) ; VESA Video Modes

	public	VESAOEMString
VESAOEMString	db  81	dup ( 0 )   ; VESA OEM String

    public  gran_shifts, isolate_bank_shifts, isolate_window_offset
    public  anti_isolate_window_offset
    public  inc_isolate_window_offset
    public  pitch
    public  read_window, write_window, bytes_in_window, num_windows
    public  bytes_left_over, banks_per_window

gran_shifts	dw  0		    ; # of shifts to adjust granularity
				    ; of 'set_bank's

isolate_bank_shifts   dd 16	    ; # of shifts to isolate the bank from a
				    ; video address

isolate_window_offset dd 0ffffh     ; mask to isolate the video window offset
anti_isolate_window_offset dd 0ffff0000h
inc_isolate_window_offset dd 10000h

pitch		dd  640 	    ; current bytes per scanline

real_buf_seg	dw  0		    ; contains real mode paragraph address

    public  write_bank, read_bank
write_bank	dw  0		    ; write window bank in '_set_write_bank'
read_bank	dw  0		    ; read window bank in '_set_read_bank'

read_window	dw  0		    ; read window offset from bottom of memory
write_window	dw  0		    ; write window offset from bottom of memory

bytes_in_window dd  0		    ; number of bytes in a window

    public  read_vidbuf
    public  write_vidbuf
read_vidbuf	dd  0		    ; offset from bottom of memory
write_vidbuf	dd  0		    ; offset from bottom of memory

num_windows	dd  0		    ; see 'set_rast()', below
bytes_left_over dd  0		    ; "          "          "
banks_per_window dd  0		    ; "          "          "

bit_rotate_val	db  80h
bit_plane_loc	dd  0

mask_bit	db  80h
x_start 	dd  0
x_count 	dd  0


parblk	struc
pbint	dw	10h		; Interrupt to issue
pbds	dw	0		; DS value
pbes	dw	0		; ES value
pbfs	dw	0		; FS value
pbgs	dw	0		; GS value
pbeax	dd	0		; eax value
pbedx	dd	0		; edx value
parblk	ends

pb	parblk	<>		; Only instance of this structure.

_data	ends

;*****************************************************************************

_text	segment

	assume	cs:CGROUP, ds:DGROUP

;*****************************************************************************
;
;   Errcode is_it_compaq();
;
;	INPUT:	    none
;	OUTPUT:     Success if a VESA card, else Failure if not a VESA card
;
;*****************************************************************************

is_it_compaq  proc    near

	public	is_it_compaq


	push	ebx
	push	ecx

	mov	eax, 0bf03h		; get info in ECX bit 40h whether
	xor	ebx, ebx		; mode 2eh is supported
	xor	ecx, ecx
	call	int10

	mov	ebx, offset VESAModes	; mode list is:  13h, 2eh, -1
	mov	word ptr [ebx], 13h
	mov	word ptr [ebx+2], 2eh
	mov	word ptr [ebx+4], -1

	test	ecx, 40h		; if mode 2eh not supported,
	jnz	#after_2eh_test
	mov	word ptr [ebx+2], -1	; then remove 2eh mode from the list
#after_2eh_test:

	mov	eax, 0			; success

	pop	ecx
	pop	ebx


	ret

is_it_compaq  endp

;*****************************************************************************
;	INT10
;
;	This routine just issues an INT 10 (BIOS Video) function, using
;	DOS Extender's function for arbitrary real-mode interrupts.
;	Necessary only for those BIOS functions not directly emulated
;	(interfaced to, I suppose) by DOS Extender.  Damages no registers
;	except what the requested INT does.
;
;
;	Parameter block for DOS Extender's function to issue a general
;	real-mode interrupt (INT 21h, AX=2511h):
;
;*****************************************************************************


int10	proc	near

	mov	pb.pbint, 10h
	mov	pb.pbeax, eax	; Store caller's eax and edx in
	mov	pb.pbedx, edx	;    parameter block.
	mov	ax,2511h	; DOS Extender function
	lea	edx,pb		; Pointer to parameter block
	int	21h		; Issue the interrupt.
	mov	edx,pb.pbedx
	cld			; Let's be paranoid.
	ret

int10	endp

;*****************************************************************************

set_xmode   proc    near

	public	set_xmode

	push	ebp
	mov	ebp, esp
	push	ebx
	push	ecx
	push	edx

	mov	eax, 0			; set video mode 'args'
	mov	al, args
	call	int10

	cmp	dword ptr args, 13h	; only set vid env if >= mode 13h
	jb	sx_exit

	mov	dx, 3ceh		; unlock the extended registers
	mov	al, 0fh
	out	dx, al
	mov	dx, 3cfh
	mov	al, 5
	out	dx, al

	cmp	dword ptr args, 2eh
	jne	sx_after_2eh


	mov	dx, 3ceh		; packed pixel, mode 2eh enabled
	mov	al, 40h 		; 1M wraparound for BITBLT
	out	dx, al
	mov	dx, 3cfh
	mov	al, 41h
	out	dx, al

	mov	dx, 3ceh		; use 64K banks
	mov	al, 6
	out	dx, al
	mov	dx, 3cfh
	in	al, dx
	and	al, 0f3h
	out	dx, al

sx_after_2eh:

	mov	ecx, args
	call	_get_mode_info

	call	set_video_environment

sx_exit:
	pop	edx
	pop	ecx
	pop	ebx
	pop	ebp
	ret

set_xmode   endp

;*****************************************************************************

get_xmode   proc    near

	public	get_xmode


	push	ebx
	push	edx

	mov	eax, 0f00h
	call	int10

	and	eax, 0ffh

	pop	edx
	pop	ebx
	ret

get_xmode   endp

;*****************************************************************************
;
;	INPUT:	    CX == Video Mode
;
;*****************************************************************************

get_mode_info	proc	near

	public	get_mode_info

	push	ebp
	mov	ebp, esp
	push	ecx

	mov	ecx, args
	call	_get_mode_info

	pop	ecx
	pop	ebp
	ret

get_mode_info	endp

_get_mode_info	proc	near

	push	esi
	mov	esi, offset VESAModeBlock

	mov	[esi].ms_modeattr, 0
	mov	[esi].ms_winAattr, CAN_READ_WRITE
	mov	[esi].ms_winBattr, CAN_READ_WRITE
	mov	[esi].ms_winsize, 64
	mov	[esi].ms_wingran, 4
	mov	[esi].ms_winAseg, 0a000h
	mov	[esi].ms_winBseg, 0a000h
	mov	[esi].ms_bitsperpixel, 8
	mov	[esi].ms_xcharsize, 8
	mov	[esi].ms_ycharsize, 16
	mov	[esi].ms_memmodel, PACKED_PIXEL

	cmp	ecx, 2eh
	jne	gmi_13h
					; mode 2eh (640x480x256)

	mov	[esi].ms_modeattr, 01bh
	mov	[esi].ms_pitch, 1024
	mov	[esi].ms_xsize, 640
	mov	[esi].ms_ysize, 480

	jmp	gmi_exit

gmi_13h:
	cmp	ecx, 13h
	jne	gmi_exit
					; mode 13h (320x200x256)

	mov	[esi].ms_modeattr, 01bh
	mov	[esi].ms_pitch, 320
	mov	[esi].ms_xsize, 320
	mov	[esi].ms_ysize, 200

gmi_exit:
	pop	esi
	ret

_get_mode_info	endp

;*****************************************************************************

set_video_environment  proc    near

	push	ebx
	push	ecx
	push	edx

	;   check which windows can read, write, or both
	;   remember what their segments are

	mov	al, VESAModeBlock.ms_winAattr
	and	al, CAN_READ_WRITE
	mov	ah, VESAModeBlock.ms_winBattr
	and	ah, CAN_READ_WRITE

	cmp	al, CAN_READ_WRITE
	jne	sve_1
				    ; window A reads & writes
	mov	read_window, 0
	movzx	edx, VESAModeBlock.ms_winAseg
	mov	read_vidbuf, edx

	mov	write_window, 0
	movzx	edx, VESAModeBlock.ms_winAseg
	mov	write_vidbuf, edx

	jmp	sve_windows_checked
sve_1:
	cmp	ah, CAN_READ_WRITE
	jne	sve_2
				    ; window B reads & writes
	mov	read_window, 1
	movzx	edx, VESAModeBlock.ms_winBseg
	mov	read_vidbuf, edx

	mov	write_window, 1
	movzx	edx, VESAModeBlock.ms_winBseg
	mov	write_vidbuf, edx

	jmp	sve_windows_checked
sve_2:
	cmp	al, CAN_READ
	jne	sve_3
					; window A reads, window B writes
	mov	read_window, 0
	movzx	edx, VESAModeBlock.ms_winAseg
	mov	read_vidbuf, edx

	mov	write_window, 1
	movzx	edx, VESAModeBlock.ms_winBseg
	mov	write_vidbuf, edx

	jmp	sve_windows_checked
sve_3:
					; window B reads, window A writes
	mov	read_window, 1
	movzx	edx, VESAModeBlock.ms_winBseg
	mov	read_vidbuf, edx

	mov	write_window, 0
	movzx	edx, VESAModeBlock.ms_winAseg
	mov	write_vidbuf, edx

sve_windows_checked:

	;   convert a real-mode segment value to a real-mode offset

	shl	read_vidbuf, 4
	shl	write_vidbuf, 4

	;   compute the number of 'bytes_in_window'

	movzx	eax, VESAModeBlock.ms_winsize
	shl	eax, 10
	mov	bytes_in_window, eax

	;   compute 'gran_shifts', the number of left-shifts of the number x
	;   that will produce a bank number that will move the window view
	;   to window x.

	mov	dx, 0
	mov	ax, VESAModeBlock.ms_winsize
	div	VESAModeBlock.ms_wingran
	bsf	bx, ax
	mov	gran_shifts, bx

	;   compute 'isolate_bank_shifts', this many right-shifts on the
	;   video address to isolate the bank

	mov	ax, VESAModeBlock.ms_winsize
	bsf	bx, ax
	add	bx, 10
	mov	word ptr isolate_bank_shifts, bx

	;   compute 'isolate_window_offset', this masks the video address
	;   leaving the offset from the start of the window

	mov	ecx, isolate_bank_shifts
	mov	eax, 1
	shl	eax, cl
	mov	inc_isolate_window_offset, eax
	dec	eax
	mov	isolate_window_offset, eax
	not	eax
	mov	anti_isolate_window_offset, eax

	movzx	edx, VESAModeBlock.ms_pitch
	mov	pitch, edx

	;   compute the number of windows to flood a screen, how many
	;   bytes to be filled in the last window, and how many banks
	;   per window (these are all used in 'set_rast()', below)

	movzx	eax, VESAModeBlock.ms_ysize
	mov	ebx, pitch
	imul	eax, ebx
	xor	edx, edx		; EDX:EAX == bytes to top of screen

	mov	ebx, bytes_in_window	; EBX == number of bytes in a window

	div	ebx
	mov	num_windows, EAX	; number of windows to flood completely
	mov	bytes_left_over, EDX	; number of bytes in the last window

	mov	eax, inc_isolate_window_offset
	mov	ecx, isolate_bank_shifts
	shr	eax, cl
	mov	banks_per_window, eax	; how many banks in a window


	;   set both banks to bottom of video buffer

	mov	edx, 0
	call	_set_write_bank
	mov	edx, 0
	call	_set_read_bank

	pop	edx
	pop	ecx
	pop	ebx
	ret

set_video_environment  endp

;*****************************************************************************

_set_read_bank	 proc	 near

	public	_set_read_bank

	push	eax
	push	ebx
	push	ecx
	push	edx

	mov	bx, dx
	mov	read_bank, bx	    ; remember this read bank
	mov	ax, read_window     ; if same read/write,
	cmp	ax, write_window
	jnz	_srb_bank_values_set
	mov	write_bank, bx	    ; remember this as write bank, too
_srb_bank_values_set:
	and	ebx, 0ffffh
	mov	cx, gran_shifts
	shl	ebx, cl

	mov	dx, 3ceh	    ; page 1 bank select
	mov	al, 45h
	out	dx, al
	mov	dx, 3cfh
	mov	al, bl
	out	dx, al

	pop	edx
	pop	ecx
	pop	ebx
	pop	eax

	ret

_set_read_bank	 endp

;*****************************************************************************

_set_write_bank   proc	  near

	public	_set_write_bank

	push	eax
	push	ebx
	push	ecx
	push	edx

	mov	bx, dx
	mov	write_bank, bx	    ; remember which bank last written to

	mov	ax, read_window     ; if same read/write window
	cmp	ax, write_window
	jnz	_swb_bank_values_set
	mov	read_bank, bx	    ; remember this for read bank, too
_swb_bank_values_set:

	and	ebx, 0ffffh
	mov	cx, gran_shifts
	shl	ebx, cl

	mov	dx, 3ceh	    ; page 1 bank select
	mov	al, 45h
	out	dx, al
	mov	dx, 3cfh
	mov	al, bl
	out	dx, al

	pop	edx
	pop	ecx
	pop	ebx
	pop	eax

	ret

_set_write_bank   endp

;*****************************************************************************

set_read_bank	proc	near

	cmp	dx, read_bank		; if not in the correct read bank
	je	srb_exit
	call	_set_read_bank
srb_exit:
	ret

set_read_bank	endp

;*****************************************************************************

set_write_bank	 proc	 near

	cmp	dx, write_bank		; if not in the correct write bank
	je	swb_exit
	call	_set_write_bank
swb_exit:
	ret

set_write_bank	 endp

;*****************************************************************************

set_read_write_bank proc   near

	cmp	dx, read_bank		; if not in the correct read bank
	je	srwb_write_bank
	call	_set_read_bank		; change to the correct read bank
srwb_write_bank:
	cmp	dx, write_bank		; if not in the correct write bank
	je	srwb_exit
	call	_set_write_bank 	; change to the correct write bank
srwb_exit:
	ret

set_read_write_bank endp

;*****************************************************************************
;
;   void    wait_vblank(VRaster *r);
;
;*****************************************************************************

wait_vsync  proc near

	public	wait_vsync


	mov	dx, 3dah	    ; video status port

#wait_loop:
	in	al, dx
	test	al, 8
	jz	#wait_loop

	ret

wait_vsync  endp


;*****************************************************************************
;
;	/* NOTE:  the color table has values that range from 0 to 255,
;		  so the colors must be divided by 4 so they'll fit into
;		  VGA tables that only range from 0 to 63 */
;
;	void set_colors(VRaster *r, LONG start, LONG len, void *table)
;
;*****************************************************************************

set_colors proc    near

	public	set_colors

	push	ebp
	mov	ebp, esp
	push	ebx
	push	ecx
	push	edx
	push	edi
	push	esi

	mov	ebx, args+4	    ; starting color #
	mov	edi, args+8	    ; number of colors
	mov	esi, args+12	    ; pallette address

#sc_loop:
	mov	edx, 3c8h	    ; vga color control port
	mov	eax, ebx	    ; current color #

	out	dx, al		    ; select color
	inc	dx		    ; vga color palette port

	jmp	#1
#1:
	lodsb
	shr	al, 2		    ; convert 256- to 64-level color
	out	dx, al		    ; red

	jmp	#2
#2:
	lodsb
	shr	al, 2		    ; convert 256- to 64-level color
	out	dx, al		    ; green

	jmp	#3
#3:
	lodsb
	shr	al, 2		    ; convert 256- to 64-level color
	out	dx, al		    ; blue

	inc	ebx		    ; next color #

	dec	edi
	jnz	#sc_loop

	pop esi
	pop edi
	pop edx
	pop ecx
	pop ebx
	pop ebp

	ret

set_colors endp

;******************************************************************************

_text	ends

	end


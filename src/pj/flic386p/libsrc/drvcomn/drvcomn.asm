;*****************************************************************************
;* DRVCOMN.ASM - VGA hardware interfaces and common driver support routines.
;*
;*  NOTES:
;*
;*	This code is now shared by more than one driver!  Be ultra-careful
;*	before changing anything like which parms are passed in registers
;*	and which registers are preserved for internal service routines such
;*	as _pj_vdrv_build_ytable.  If you need to find everyone that uses this
;*	code, grep for DRVCOMN.LIB in all the MAKEFILEs and/or *.LNK files
;*	in all subdirs of \paa\drivekit.
;*
;*	Within the comment blocks for each routine below, if the first line
;*	of comments looks like a C-language prototype, the routine is called
;*	from C, and the normal Watcom -3s parm and register conventions apply.
;*	If the first line doesn't look like a prototype, it's an internal
;*	service routine (called from other asm code), and the parms/regs are
;*	listed in the comment block.
;*
;*  MAINTENANCE:
;*    ??/??/91	Jim Byers
;*		Original version (hardware.asm).
;*    03/27/91	Ian Lepore
;*		Overall driver was rewritten.  The code in this module was
;*		mostly copied from the original driver's hardware.asm module.
;*		All the output routines were removed to their own modules,
;*		then code was added herein to build the ytable during mode
;*		initialization.  Other small changes were made to glue the
;*		original hardware.asm code to changes in device.c.
;*    04/25/91	Ian Lepore
;*		Changed the pj_vdrv_set_colors routine (see comments there),
;*		and added pj_vdrv_uncc64 and pj_vdrv_uncc256.
;*    05/28/91	Ian
;*		New module, based on routines formerly in VESAINTF.ASM, now
;*		isolated here as 'common to all SVGA cards' code.
;*    06/04/91	Ian
;*		Added pj_vdrv_8bit_set_colors() and pj_vdrv_8bit_uncc256() to support
;*		svga cards with full 8-bit dacs.
;*    07/30/91	Ian
;*		Added declaration of pj_vdrv_modeinfo below; this is now used
;*		by the vesa and svga drivers.
;*    08/20/91	Ian
;*		Moved declaration of pj_vdrv_raster_library into this module.
;*		This is because there should only be one of these, ideally,
;*		and having them declared in the device.c modules of the users
;*		of the common driver routines leads to multiple declarations
;*		of it in the flilib code.
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
;******************************************************************************

	include stdmacro.i
	include drvcomn.i
	include rastlib.i

;*****************************************************************************
;* the pj_vdrv_has_8bitdac variable.
;* this is init'd to FALSE, but any detect/init routine can set it to true
;* as an indication to the build_rastlib routine that the 8bit color-setting
;* code should be used instead of the 6bit routines.
;*****************************************************************************

_DATA	segment

	public	pj_vdrv_has_8bitdac

pj_vdrv_has_8bitdac dd 0		; init 8bitdac flag to FALSE.

_DATA	ends

;*****************************************************************************
;* the sminfo, ytable, and wcontrol structures.
;*****************************************************************************

_BSS	segment

	public	pj_vdrv_raster_library
	public	pj_vdrv_modeinfo
	public	pj_vdrv_wcontrol
	public	pj_vdrv_ytable

;-----------------------------------------------------------------------------
; the raster libary structure...
;-----------------------------------------------------------------------------

pj_vdrv_raster_library dd RL_NUM_LIB_CALLS dup (?)

;-----------------------------------------------------------------------------
; the sminfo data structure...
;   this is used to relate our internal mode numbers to the hardware modes.
;-----------------------------------------------------------------------------

	align 2

pj_vdrv_modeinfo sminfo MAX_SMODES dup (<>)  ; define an array of sminfo structs.

;-----------------------------------------------------------------------------
; the ytable data structure...
;
;  The whole show is built around this data structure, so a few words are
;  probably in order.  While the table is defined here as an array of dwords,
;  and is most frequently accessed that way, it is conceptually defined as:
;
;    struct ytable {
;      short  split_at
;      short  bank_num
;      long   offset
;      };
;
;  We allocate an array of 1536 (1.5k) of these structures.  (Code in the
;  open_graphics() routine will prevent opening a screen with more than 1536
;  scanlines).	It is important for performance that this array be statically
;  allocated; dynamic allocation at runtime is not an option.  The data in
;  the table is generated by the set_video_environment routine, below.
;
;  All screen-access code in the driver uses this table as part of addressing
;  video memory.  Typically, the Y coordinate is used to index to the proper
;  table entry, and the split_at/bank_num values are obtained with a single
;  dword move.	The split/bank value is then compared to the current bank
;  stored in the window control structure.  If they are not equal, a branch
;  is taken to a special handling routine for further action.  If they are
;  equal, the offset is obtained from the table, and screen access commences.
;  The following code fragment illustrates this:
;
;      move  edi,y_coordinate		; load the Y coordinate for the line.
;      mov   eax,[edi*8+ytab_bank]	; load split_at/bank_num for the line,
;      cmp   eax,pj_vdrv_wcontrol.wwrcurbank	; compare to the current bank number.
;      jne   check_split		; if split line or wrong bank, go do
;      mov   edi,[edi*8+ytab_offs]	; special handling, else get the line
;      ; do output			; offset and proceed with the access.
;
;  When a bank switch is done, the split_at value is NOT stored in the
;  wwrcurbank variable, so a comparison such as that shown above will take
;  the branch when the target line is in a bank other than the current one
;  (ie, a bank switch is needed), or when the target line has a split in it.
;  The premise behind this is that split lines and bank switches are only
;  rarely needed - most output will be to an unsplit line in the current bank.
;  (This is because we mostly process from the top of the screen downwards,
;  truely random accesses to the screen are pretty rare.)  Thus, the logic
;  shown above tests for both of the exceptional conditions with a single test
;  and branch, and the fall-thru case will be taken most of the time (avoiding
;  the nemisis of 386 performance coding: taking branches).
;
;  This table MUST be aligned on a dword boundry for performance!!!
;-----------------------------------------------------------------------------

	align 4

pj_vdrv_ytable	dd 1536*2 dup (?)	   ; ytable, entries are 2 dwords each

;-----------------------------------------------------------------------------
; the pj_vdrv_wcontrol data structure....
;
;  This structure is described in detail in DRVCOMN.I where it is defined. It
;  is statically allocated to allow fast access to it via LEA (2 cycles).
;
;  This structure MUST be aligned on a dword boundry for performance!!!
;-----------------------------------------------------------------------------

	align 4

pj_vdrv_wcontrol      winblk  <>		; window control block

_BSS	ends

;*****************************************************************************
;* code...
;*****************************************************************************

_TEXT	segment

	public	_pj_vdrv_build_ytable
	public	pj_vdrv_wait_vblank
	public	pj_vdrv_set_colors
	public	pj_vdrv_uncc256
	public	pj_vdrv_uncc64
	public	pj_vdrv_8bit_set_colors
	public	pj_vdrv_8bit_uncc256
	public	pj_vdrv_8bit_uncc64

;*****************************************************************************
;* _pj_vdrv_build_ytable - internal service routine, calc's and fills in ytable.
;*
;* Entry:
;*   following values in pj_vdrv_wcontrol struct must be valid:
;*	wpitch wheight wbankshift woffsmask
;* Exit:
;*   all registers preserved.
;*****************************************************************************

_pj_vdrv_build_ytable proc near

	pushad

	lea	esi,pj_vdrv_ytable		; ptr for output of values to ytable.
	lea	edi,pj_vdrv_wcontrol		; ptr to pj_vdrv_wcontrol, we use it a lot.
	mov	edx,[edi].wheight	; loop counter, lines in ytable.
	mov	cl,[edi].wbankshift	; we'll be shifting a lot, preload this.
	xor	ebx,ebx 		; zero starting window offset.
#make_ytable:
	mov	eax,ebx 		; get offset of current line from the
	and	eax,[edi].woffsmask	; start of the screen, mask it to an
	mov	[esi+4],eax		; offset from start of window, store it.
	mov	eax,ebx 		; get offset of current line, shift it
	shr	eax,cl			; to get the bank number.  Save the
	push	eax			; bank number on the stack for now.
	add	ebx,[edi].wpitch	; increment offset to next screen line.
	lea	eax,[ebx-1]		; calc the address of the last byte on
	shr	eax,cl			; this line, then get the bank number
	cmp	eax,[esp]		; of that address.  compare that to the
	je	short #nosplit		; bank number for the start of the
	mov	eax,[edi].woffsmask	; line.  if they're not equal, the
	inc	eax			; line is split.  calc the location
	sub	eax,[esi+4]		; of the split and store it alongside
	mov	[esp+2],ax		; the bank number saved on the stack.
#nosplit:
	pop	eax			; get the split_at/bank_num value,
	mov	[esi],eax		; store it into the ytable.
	add	esi,8			; point to the next ytable entry.
	dec	edx			; decrement the loop counter, if not
	jnz	short #make_ytable	; zero, continue with the next line.

	popad
	ret

_pj_vdrv_build_ytable endp

;*****************************************************************************
;* void pj_vdrv_wait_vblank(void)
;*  busy-wait for vblank (this works with any VGA)
;*****************************************************************************

	align 4
pj_vdrv_wait_vblank proc near

	mov	edx,3dah		; video status port
#wait:
	in	al,dx
	test	al,8
	jz	short #wait
	ret

pj_vdrv_wait_vblank endp

;*****************************************************************************
; void pj_vdrv_set_colors(Raster *r, int start, int count, UBYTE *color_table)
;
;	/* NOTE:  the color table has values that range from 0 to 255,
;		  so the colors must be divided by 4 so they'll fit into
;		  VGA tables that only range from 0 to 63 */
;
; Note, this routine modified as follows:
;
;	My VGA book says that a minimum of 240ns must elapse between accesses
;	to the vga color data registers.  Assuming a 50mhz 386 chip, this
;	means we need 12 cycles between OUT instructions when setting colors.
;	The original version used JMP instructions to add a delay, and this
;	was caused delay between accesses of about 15-16 cycles plus the
;	time to reload the prefetch queue.  Now the routine uses NOP
;	instructions to effect the slight delay needed.  (One NOP does the
;	the trick (just barely), giving us 11 cycles between accesses.	The
;	use of 11 instead of 12 is predicated on Jim K's experience of never
;	finding a card that really needs a delay at all, and the fact that we
;	already have a huge fudge factor built in by assuming a 50mhz 386
;	running without any wait states while accessing either memory or
;	the graphics card.
;
;	Also, the original routine re-selected the next color register
;	after each rgb triplet, but my vga book says that the
;	register number will auto-increment after each triplet, and it
;	doesn't mention any exceptions to that for given cards.
;
;	Anyway, if any of the above turns out to be bogus for a given card,
;	the original routine still exists below, commented out with a IF 0.
;
;*****************************************************************************

	align 4
pj_vdrv_set_colors proc    near

#raster equ	[esp+4]
#start	equ	[esp+08]
#count	equ	[esp+12]
#ptable equ	[esp+16]

	mov	eax,#start		; starting color #
	mov	ecx,#count		; number of colors
	mov	edx,#ptable		; pallette address

	push	esi
	mov	esi,edx

	mov	edx,3c8h		; address vga color palette control port
	out	dx,al			; select starting color register
	inc	edx			; address vga color palette data port
	nop
#loop:
	lodsb				; load red
	shr	al,2			; convert 256- to 64-level color
	out	dx,al			; store red
	nop				; waste 3 cycles
	lodsb				; load green
	shr	al,2			; convert 256- to 64-level color
	out	dx,al			; store green
	nop				; waste 3 cycles
	lodsb				; load blue
	shr	al,2			; convert 256- to 64-level color
	out	dx,al			; store blue
	dec	ecx			; count the color as done
	jnz	short #loop		; go do next color (wastes 8+ cycles)

	pop esi
	ret

pj_vdrv_set_colors endp

 if 0	; old version...

	align 4
pj_vdrv_set_colors proc    near

#raster equ	[ebp+8]
#start	equ	[ebp+12]
#count	equ	[ebp+16]
#ptable equ	[ebp+20]

	push	ebp
	mov	ebp, esp
	push	ebx
	push	esi

	mov	ebx,#start		; starting color #
	mov	ecx,#count		; number of colors
	mov	esi,#ptable		; pallette address

#loop:
	mov	edx,3c8h		; vga color control port
	mov	al,bl			; current color #

	out	dx,al			; select color
	inc	edx			; vga color palette port
	jmp	#delay1
#delay1:
	lodsb
	shr	al,2			; convert 256- to 64-level color
	out	dx,al			; red
	jmp	#delay2
#delay2:
	lodsb
	shr	al,2			; convert 256- to 64-level color
	out	dx,al			; green
	jmp	#delay3
#delay3:
	lodsb
	shr	al,2			; convert 256- to 64-level color
	out	dx,al			; blue

	inc	ebx			; next color #

	loop	#loop

	pop esi
	pop ebx
	pop ebp

	ret

pj_vdrv_set_colors endp

 endif

;*****************************************************************************
; void pj_vdrv_uncc256(Raster *r, void *ucbuf);
;
; Notes under pj_vdrv_set_colors apply to this routine.
;*****************************************************************************

	align 4
pj_vdrv_uncc256 proc	near

#raster equ	[esp+4]
#ucbuf	equ	[esp+8]

	mov	edx,#ucbuf
	push	ebx
	push	esi
	push	edi
	mov	esi,edx

	movzx	ebx,word ptr [esi]
	add	esi,2
	xor	edi,edi
	xor	ecx,ecx
#packetloop:
	dec	ebx
	js	short #done
	mov	cl,[esi]		; get skip count
	add	edi,ecx 		; add to color index
	mov	cl,[esi+1]		; get change count
	add	esi,2			; incr input pointer

	mov	eax,edi 		; load color register index
	mov	edx,3c8h		; address vga color palette control port
	out	dx,al			; select starting color register
	inc	edx			; address vga color palette data port
	nop
#loop:
	inc	edi			; increment index
	lodsb				; load red
	shr	al,2			; convert 256- to 64-level color
	out	dx,al			; store red
	nop				; waste 3 cycles
	lodsb				; load green
	shr	al,2			; convert 256- to 64-level color
	out	dx,al			; store green
	nop				; waste 3 cycles
	lodsb				; load blue
	shr	al,2			; convert 256- to 64-level color
	out	dx,al			; store blue
	dec	cl			; (note: MUST dec cl, not ecx!)
	jnz	short #loop		; go do next color (wastes 8+ cycles)
	jmp	short #packetloop
#done:
	pop	edi
	pop	esi
	pop	ebx
	ret

pj_vdrv_uncc256 endp

;*****************************************************************************
; void pj_vdrv_uncc64(Raster *r, void *ucbuf);
;
; Notes under pj_vdrv_set_colors apply to this routine.
; This is an exact clone of uncc256, except the 256-to-64-level shift
; instructions were replaced with NOPs.
;*****************************************************************************

	align 4
pj_vdrv_uncc64 proc    near

#raster equ	[esp+4]
#ucbuf	equ	[esp+8]

	mov	edx,#ucbuf
	push	ebx
	push	esi
	push	edi
	mov	esi,edx

	movzx	ebx,word ptr [esi]
	add	esi,2
	xor	edi,edi
	xor	ecx,ecx
#packetloop:
	dec	ebx
	js	short #done
	mov	cl,[esi]		; get skip count
	add	edi,ecx 		; add to color index
	mov	cl,[esi+1]		; get change count
	add	esi,2			; incr input pointer

	mov	eax,edi 		; load color register index
	mov	edx,3c8h		; address vga color palette control port
	out	dx,al			; select starting color register
	inc	edx			; address vga color palette data port
	nop
#loop:
	inc	edi			; increment index
	lodsb				; load red
	nop				; color is already 64-level
	out	dx,al			; store red
	nop				; waste 3 cycles
	lodsb				; load green
	nop				; color is already 64-level
	out	dx,al			; store green
	nop				; waste 3 cycles
	lodsb				; load blue
	nop				; color is already 64-level
	out	dx,al			; store blue
	dec	cl			; (note: MUST dec cl, not ecx!)
	jnz	short #loop		; go do next color (wastes 8+ cycles)
	jmp	short #packetloop
#done:
	pop	edi
	pop	esi
	pop	ebx
	ret

pj_vdrv_uncc64 endp

;*****************************************************************************
; void pj_vdrv_8bit_set_colors(Raster *r, int start, int count, UBYTE *color_table)
;
;   Set colors routine for cards with 8-bit DACs.
;*****************************************************************************

	align 4
pj_vdrv_8bit_set_colors proc	near

#raster equ	[esp+4]
#start	equ	[esp+08]
#count	equ	[esp+12]
#ptable equ	[esp+16]

	mov	eax,#start		; starting color #
	mov	ecx,#count		; number of colors
	mov	edx,#ptable		; pallette address

	push	esi
	mov	esi,edx

	mov	edx,3c8h		; address vga color palette control port
	out	dx,al			; select starting color register
	inc	edx			; address vga color palette data port
	nop
#loop:
	lodsb				; load red
	nop				;
	out	dx,al			; store red
	nop				; waste 3 cycles
	lodsb				; load green
	nop				;
	out	dx,al			; store green
	nop				; waste 3 cycles
	lodsb				; load blue
	nop				;
	out	dx,al			; store blue
	dec	ecx			; count the color as done
	jnz	short #loop		; go do next color (wastes 8+ cycles)

	pop esi
	ret

pj_vdrv_8bit_set_colors endp

;*****************************************************************************
; void pj_vdrv_8bit_uncc256(Raster *r, void *ucbuf);
;
;*****************************************************************************

	align 4
pj_vdrv_8bit_uncc256 proc	near

#raster equ	[esp+4]
#ucbuf	equ	[esp+8]

	mov	edx,#ucbuf
	push	ebx
	push	esi
	push	edi
	mov	esi,edx

	movzx	ebx,word ptr [esi]
	add	esi,2
	xor	edi,edi
	xor	ecx,ecx
#packetloop:
	dec	ebx
	js	short #done
	mov	cl,[esi]		; get skip count
	add	edi,ecx 		; add to color index
	mov	cl,[esi+1]		; get change count
	add	esi,2			; incr input pointer

	mov	eax,edi 		; load color register index
	mov	edx,3c8h		; address vga color palette control port
	out	dx,al			; select starting color register
	inc	edx			; address vga color palette data port
	nop
#loop:
	inc	edi			; increment index
	lodsb				; load red
	nop				;
	out	dx,al			; store red
	nop				; waste 3 cycles
	lodsb				; load green
	nop				;
	out	dx,al			; store green
	nop				; waste 3 cycles
	lodsb				; load blue
	nop				;
	out	dx,al			; store blue
	dec	cl			; (note: MUST dec cl, not ecx!)
	jnz	short #loop		; go do next color (wastes 8+ cycles)
	jmp	short #packetloop
#done:
	pop	edi
	pop	esi
	pop	ebx
	ret

pj_vdrv_8bit_uncc256 endp

;*****************************************************************************
; void pj_vdrv_8bit_uncc64(Raster *r, void *ucbuf);
;*****************************************************************************

	align 4
pj_vdrv_8bit_uncc64 proc    near

#raster equ	[esp+4]
#ucbuf	equ	[esp+8]

	mov	edx,#ucbuf
	push	ebx
	push	esi
	push	edi
	mov	esi,edx

	movzx	ebx,word ptr [esi]
	add	esi,2
	xor	edi,edi
	xor	ecx,ecx
#packetloop:
	dec	ebx
	js	short #done
	mov	cl,[esi]		; get skip count
	add	edi,ecx 		; add to color index
	mov	cl,[esi+1]		; get change count
	add	esi,2			; incr input pointer

	mov	eax,edi 		; load color register index
	mov	edx,3c8h		; address vga color palette control port
	out	dx,al			; select starting color register
	inc	edx			; address vga color palette data port
	nop
#loop:
	inc	edi			; increment index
	lodsb				; load red
	shl	al,2			; make 64-level color into 256-level
	out	dx,al			; store red
	nop				; waste 3 cycles
	lodsb				; load green
	shl	al,2			; make 64-level color into 256-level
	out	dx,al			; store green
	nop				; waste 3 cycles
	lodsb				; load blue
	shl	al,2			; make 64-level color into 256-level
	out	dx,al			; store blue
	dec	cl			; (note: MUST dec cl, not ecx!)
	jnz	short #loop		; go do next color (wastes 8+ cycles)
	jmp	short #packetloop
#done:
	pop	edi
	pop	esi
	pop	ebx
	ret

pj_vdrv_8bit_uncc64 endp

_TEXT	ends
	end

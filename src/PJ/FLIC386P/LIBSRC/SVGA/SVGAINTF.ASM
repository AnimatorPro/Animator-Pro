;*****************************************************************************
;* SVGAINTF.ASM - The hardware interface code for the supervga driver.
;*
;* NOTES:
;*
;*   This driver is designed to work with most super vga cards that have
;*   a windowed access method into video memory.  (Any card that could
;*   support a VESA BIOS could also be supported in this driver).  The
;*   bulk of the driver is contained in the 'common driver routines'.
;*   As of this writing, those routines live in the VESA subdirectory,
;*   and all modules have names starting with DRV.  The code in this
;*   module is primarily concerned with detecting various types of
;*   hardware, setting svga video modes, and handling bank switches for
;*   the bulk of the driver code.  Even when VESA is installed on the
;*   system, this driver will try to detect the hardware first, because
;*   there is a huge performance boost in bank switching directly as
;*   opposed to using the VESA BIOS functions.
;*
;*   This driver is ninety percent smoke-n-mirrors, so I guess I'll write
;*   a little book here about how it works.  This module exports no
;*   data items; it exports 5 direct entry points:
;*
;*	pj_svga_init	- Detect attached hardware.
;*	pj_svga_cleanup - Perform optional unload-time cleanup.
;*	pj_svga_setmode - Set a given svga video mode.
;*	pj_svga_clrmode - Unset a given mode, return to original mode.
;*	pj_svga_getname - Return a pointer to the detected device name.
;*
;*   These entry points are all invoked from the DEVICE.C module.  In
;*   addition to these, the bank switch routines in this module are
;*   invoked via indirect calls from the common driver routines.
;*
;*   The pj_svga_init routine is called by the REX-layer init vector as
;*   soon as the driver is loaded.  It is responsible for detecting the
;*   type of hardware attached to the system, and informing the
;*   higher-level routines what modes the hardware is capable of.  The
;*   detection process calls a series of detect-this-type-of-card
;*   routines.	These routines return zero if they don't recognize the
;*   hardware, so the init routine just keeps trying different hardware
;*   until something responds.	(If nothing does, it returns
;*   Err_no_display).  When a specific hardware-detect routine sees a
;*   known type of hardware, it loads the SMInfo array with the hardware
;*   mode number, width, and height of each mode that hardware can do.
;*   It also sets pointers to the appropriate setmode() and clrmode()
;*   routines for that hardware into the vsetmode and vclrmode
;*   variables, and a pointer to a string that holds the device name in
;*   the vdevname variable.  The hardware mode number in the SMInfo
;*   structure is only for use by the setmode() routine; higher-level
;*   routines ignore it.  From the width/height values loaded into the
;*   SMInfo array, the higher-level routines build our device info and
;*   text data that is provided to the host.
;*
;*   The pj_svga_cleanup routine is called by the REX-layer cleanup
;*   vector just before the driver is unloaded.  This is used primarily
;*   by the VESA part of the code, since it must free up a DOS-memory
;*   buffer that it aquires during init processing.  If any detect
;*   routine allocates resources, it can set a pointer to a cleanup
;*   routine in the vcleanup variable, and it will be called at unload
;*   time.  (Remember that this is different than device-close time.)
;*
;*   The pj_svga_setmode routine is called during device-open
;*   processing.  It simply loads a couple of parameter registers and
;*   then jumps through the vsetmode vector variable.  The routine that
;*   gets control is responsible for setting the video hardware into the
;*   requested mode so that it is ready for output.  This includes
;*   setting the bankswitch vector(s) and other mode-specific
;*   information in the pj_vdrv_wcontrol structure.  (See DRVCOMN.I for full
;*   details on the pj_vdrv_wcontrol structure.)  If at all possible, this
;*   routine should attempt to get the current video mode so that it can
;*   be restored device-close time.  To help facilitate this without
;*   10,000 global variables, the higher-level routines will remember
;*   the longword return value from the setmode() routine and will pass
;*   that value to the clrmode() routine.  This allows the old video
;*   mode to be returned to the caller, and then received at clrmode()
;*   time so that it can be restored hardware-wise.
;*
;*   The pj_svga_clrmode routine is called during device-close
;*   processing.  At this level, the routine will only be called if the
;*   device was successfully opened.  (NO-OP calls are filtered at a
;*   higher level).  This routine loads a couple of parameter registers
;*   and jumps through the vclrmode vector variable, if that variable is
;*   non-NULL.	(Yes, clrmode is optional, but highly recommended.)
;*
;*   The pj_svga_getname routine just returns the value from the
;*   vdevname variable.  That value was set during init/detect
;*   processing, and should be a string naming the device.
;*
;*   To keep the code simple <snicker>, the five entry points above save
;*   all registers that the C code callers care about, in case the
;*   vectorized routines want to trash them up a bit.  In each case, the
;*   comment blocks at the top of the routines indicate the register
;*   usage rules for that set of routines.  (Any registers means ebp can
;*   be trashed too, remember!)  (Be aware, though, that I've seen comments
;*   in other folks' source code that indicates that some video BIOSes will
;*   trash (e)bp behind your back).
;*
;*   New Feature dept:	the setmode routines can modify the contents of the
;*   rastlib structure if desired.  The open_graphics processing will fill
;*   this array in with pointers to the common driver routines before calling
;*   setmode.  The setmode can change any of the pointers in the library
;*   (via global var pj_vdrv_raster_library).  This is handy if the hardware
;*   supports some special features that can speed up given routines. (Like
;*   the V7VRAM's ability to do mask1blit/mask2blit in hardware.  Pity we
;*   don't implement it.)  Be aware that any change you make is permenant: it
;*   will not be undone automatically on clrmode.  If you want to restore
;*   the pointers to their original values upon leaving a graphics mode,
;*   you have to handle it yourself in your clrmode routine.
;*
;* MAINTENANCE:
;*   05/25/91 - Ian - New driver.
;*
;*   08/26/92 - Ian
;*	      > Removed all CLI/STI in bankswitch routines.  These instructions
;*		can take up to 300 cycles each when running under windows, and
;*		they don't even truly disable interupts due to virtualization.
;*	      > Added new error code err_video_bios.  We return it when
;*		everything looks good to us but the video bios fails to
;*		switch to the mode we request.
;*	      > Added new routines setmode_standard and clrmode_standard to
;*		handle the cards/chips that use a similar standard video
;*		bios call to set modes (ie, most svga cards).
;*	      > Video 7: detect logic changed to work under windows.  The
;*		V7 windows driver puts the card in "pure vga" mode which
;*		doesn't allow it to do svga modes.  We put the card back in
;*		extensions-allowed mode and leave it that way if detect works.
;*	      > Tseng3/Tseng4: added check to ensure mode change worked.
;*		Also, split detect of 3000/4000 into separate routines, using
;*		all-new code from vgakit.
;*	      > Trident: added additional detect logic, per the svga book.
;*		Also added memory size detect so that we only report modes
;*		for which the trident card has enough memory.
;*	      > Trident: added check to ensure mode change worked.
;*	      > ChipsTech: added check to ensure mode change worked.
;*	      > ATI: added check to ensure mode change worked.	Also, changed
;*		logic used to detect amount of memory on card, per vgakit 5.0;
;*		new logic should work with newer ATI cards.
;*	      > AheadA/B: added check to ensure mode change worked.
;*		Also, fixed bug where modeset vector wasn't being filled in
;*		for AheadB (guess no one's ever tried this chip).
;*	      > OakTech: added check to ensure mode change worked.  Also,
;*		added new better detect logic from vgakit 5.0.
;*	      > Paradise: added check to ensure mode change worked.
;*	      > Genoa: added support for this card/chipset.
;*	      > Compaq: added support for this card/chipset.
;*	      > NCR 77C22E: added support for this card/chipset.
;*****************************************************************************

	include stdmacro.i
	include drvcomn.i
	include errcodes.i

;-----------------------------------------------------------------------------
; data - our vector variables, and device name strings...
;-----------------------------------------------------------------------------

VESA_FIRST = 0	; 1 makes vesa take precedence, 0 makes it last thing tested.
		; I advise against making it first. If you do, you've got no
		; driver to work around a broken vesa bios!

_DATA	segment


	align 4
vsetmode dd	0			; -> routine to set an svga mode
vclrmode dd	0			; -> routine to unset an svga mode
vcleanup dd	0			; -> cleanup routine (at driver unload)
vdevname dd	0			; -> device name string.

chipdata dw	0			; some cards have ports that move around.
dac8bit  db	0			; dac depth, 0 = 6-bit, 1 = 8-bit

;		    1234567890123456789 ; names must be <= 19 chars (+nullterm)

vname_null	db 'Unknown device',0
vname_ativga	db 'ATI VGA',0
vname_everex	db 'Everex VGA',0
vname_evga512	db 'Everex EVGA512',0
vname_trident	db 'Trident Chipset',0
vname_video7	db 'Video 7 VRAM VGA',0
vname_video7i	db 'Video 7 i1024',0
vname_paradise	db 'Paradise VGA',0
vname_chipstech db 'Chips&Tech Chipset',0
vname_tseng3	db 'Tseng 3000 Chipset',0
vname_tseng4	db 'Tseng 4000 Chipset',0
vname_aheada	db 'Ahead Chipset ver A',0
vname_aheadb	db 'Ahead Chipset ver B',0
vname_oaktech	db 'Oak Tech Chipset',0
vname_genoa	db 'Genoa GVGA',0
vname_compaq	db 'Compaq VGA',0
vname_ncr	db 'NCR 77C22E VGA',0
vname_vesa	db 'VESA BIOS',0

_DATA	ends

;-----------------------------------------------------------------------------
; some handy macros...
;-----------------------------------------------------------------------------

ModeVec macro	setname,clrname
	mov	vsetmode,offset setmode_&setname
	ifnb <clrname>
	mov	vclrmode,offset clrmode_&clrname
	else
	mov	vclrmode,offset clrmode_&setname
	endif
	endm

NameVec macro	name
	mov	vdevname,offset vname_&name
	endm

NameChk macro	name
	cmp	vdevname,offset vname_&name
	endm

SetInfo macro	mode,width,height
	mov	[esi].smi_mode,mode
	mov	[esi].smi_width,width
	mov	[esi].smi_height,height
	add	esi,SIZE sminfo
	endm

BankVec macro	readbank,writebank
	mov	pj_vdrv_wcontrol.wsrbvector,offset setbank_&readbank
	ifnb <writebank>
	mov	pj_vdrv_wcontrol.wswbvector,offset setbank_&writebank
	else
	mov	pj_vdrv_wcontrol.wswbvector,offset setbank_&readbank
	endif
	endm

TestFor macro	name
	call	is_it_&name
	test	eax,eax
	jnz	short #done
	endm

Int10	macro	axval,bxval,cxval
	ifnb <axval>
	mov	ax,&axval
	endif
	ifnb <bxval>
	mov	bx,&bxval
	endif
	ifnb <cxval>
	mov	cx,&cxval
	endif
	push	esi
	push	edi
	int 10h
	pop	edi
	pop	esi
	endm

;-----------------------------------------------------------------------------
; code...
;-----------------------------------------------------------------------------

_TEXT	segment
	assume	cs:CGROUP,ds:DGROUP

	public	pj_svga_init
	public	pj_svga_cleanup
	public	pj_svga_setmode
	public	pj_svga_clrmode
	public	pj_svga_getname

	extrn	pj_vesa_detect:near
	extrn	pj_vesa_free_dosbuf:near
	extrn	pj_vesa_setmode:near
	extrn	pj_vesa_clrmode:near

;*****************************************************************************
;* wc_1x64k - internal service routine to set up wcontrol stuff for a device
;*	      that implements a single window of 64k used for reads & writes.
;*
;* requires edi->pj_vdrv_wcontrol, trashes eax,ecx,edx.
;*****************************************************************************

wc_1x64k proc near

	mov	eax,0A0000h
	mov	[edi].wrdaddr,eax	; set read/write screen addresses.
	mov	[edi].wwraddr,eax

	xor	eax,eax 		; set lotsa stuff to zero...

	mov	[edi].wrdwhich,eax	; which window reads  (0)
	mov	[edi].wwrwhich,eax	; which window writes (0)
	mov	[edi].wgranshift,al	; shiftamt to make bank# into window#
	inc	eax			; eax = 1
	mov	[edi].wsamerw,al	; same window reads & writes
	neg	eax			; eax = -1, set initial read and write
	mov	[edi].wrdcurbank,eax	; bank numbers to -1, forces bankswitch
	mov	[edi].wwrcurbank,eax	; on first screen access.

	mov	[edi].wbankshift,16	; shiftamt to make video addr into bank#

	mov	eax,0FFFFh		; set bank mask and dwords-in-bank
	mov	[edi].woffsmask,eax
	inc	eax
	shr	eax,2
	mov	[edi].wwindwords,eax

	mov	eax,[edi].wheight	; height of screen times line pitch
	mul	dptr [edi].wpitch	; gives total bytes per screen. total
	mov	ecx,[edi].wwindwords	; bytes divided by bytes per window
	shl	ecx,2			; (calc'd as dwords per window*4) gives
	div	ecx			; the count of full windows, with the
	mov	[edi].wwincount,eax	; remainder being the number of bytes
	mov	[edi].wwinlbytes,edx	; in the last window (if any).

	ret

wc_1x64k endp

;*****************************************************************************
;* setbank_????? - routines to set the video bank.
;*
;* These routines are called from the pj_vdrv_whatever common output routines, via
;* the pj_vdrv_wcontrol.wsrbvector and pj_vdrv_wcontrol.wswbvector variables.  The register
;* usage rules listed below are incontrovertable:  you'd have to rewrite the
;* entire common driver subsystem to change them.  Note that for many types
;* of hardware there is a single routine to switch banks, but some hardware
;* supports separate read and write banks.
;*
;* The following rules apply to each of these routines:
;*
;*  Entry:
;*	eax =  bank number
;*	edx -> pj_vdrv_wcontrol
;*  Exit:
;*	eax trashed, all others must be preserved.
;*	(edx may be 'preserved' by reloading &pj_vdrv_wcontrol into it)
;*****************************************************************************
;*****************************************************************************
;* clrmode_????? - routines to clear svga video mode, restore original mode.
;*
;* The following rules apply to each of these routines:
;*
;* Entry:
;*   esi - old_mode_data - the longword that setmode_????? returned.
;*   edi - pointer to pj_vdrv_wcontrol
;* Exit:
;*   any regs may be trashed as desired.
;*****************************************************************************
;*****************************************************************************
;* setmode_????? - routines to set svga video mode.
;*
;* The following rules apply to each of these routines:
;*
;* Entry:
;*   esi	- Points to SMInfo that describes mode to be set.
;*   edi	- Points to pj_vdrv_wcontrol.
;*   wcontrol	- wwidth, wheight, and wpitch have been set based on the values
;*		  in the SMInfo entry.	the setmode routine can override them
;*		  if necessary (only pitch might ever change, you would think).
;* Exit:
;*   eax	     - negative error code on failure, or any non-negative
;*		       longword on sucess.  the success longword will be
;*		       passed to the corresponding clrmode() routine on
;*		       device close.
;*   all other regs  - may be trashed with impunity.
;*   wcontrol	     - must be filled in as appropriate for the mode.
;*   ytable	     - must be filled in as appropriate for the mode.
;*****************************************************************************
;*****************************************************************************
;* is_it_????? - internal service routines to try and figure out the hardware.
;*
;* The following rules apply to each of these routines:
;*
;* Entry:
;*   esi - Points to array of 16 SMInfo slots; routine fills these in if it
;*	   recognizes the hardware it can deal with.
;* Exit:
;*     ebx,ecx,edx,edi,ebp may always be trashed if desired.
;*  If the routine recognizes the hardware:
;*     eax	- number of SMInfo slots filled (must not be zero).
;*     esi	- may be trashed if desired.
;*     vsetmode - contains a pointer to a routine to set a given svga mode
;*		  (must not be NULL).
;*     vclrmode - contains a pointer to a routine to restore the prior video
;*		  mode when the device is closed.  (at the very least, this
;*		  routine must leave the device in a BIOS-usable state.)
;*     vcleanup - contains a pointer to a routine to de-alloc memory or any
;*		  other special cleanup to be done when the driver is
;*		  unloaded.  this may be NULL if no cleanup actions are needed.
;*     vdevname - contains a pointer to a string that names the device.
;*  If the routine does not recognize the hardware:
;*     eax	- zero.
;*     esi	- must be preserved.
;*     vsetmode/vcleanup/vdevname should not be touched.
;*****************************************************************************


;-----------------------------------------------------------------------------
; a couple little utility functions used by detectors. (from vgakit 5.0)
;-----------------------------------------------------------------------------

_isport2 proc	near			; check for valid indexed i/o port,
	push	ebx			; al is index, ah is bit mask
	mov	bx,ax
	out	dx,al
	mov	ah,al
	inc	dx
	in	al,dx
	dec	dx
	xchg	al,ah
	push	ax
	mov	ax,bx
	out	dx,ax
	out	dx,al
	mov	ah,al
	inc	dx
	in	al,dx
	dec	dx
	and	al,bh
	cmp	al,bh
	jnz	short #noport
	mov	al,ah
	mov	ah,0
	out	dx,ax
	out	dx,al
	mov	ah,al
	inc	dx
	in	al,dx
	dec	dx
	and	al,bh
	cmp	al,0
#noport:
	pop	ax
	out	dx,ax
	pop	ebx
	ret
_isport2 endp

_isport1 proc	near		;check for valid i/o port, al is bit mask
	mov	ah,al
	in	al,dx
	push	ax
	mov	al,ah
	out	dx,al
	in	al,dx
	and	al,ah
	cmp	al,ah
	jnz	short #noport
	mov	al,0
	out	dx,al
	in	al,dx
	and	al,ah
	cmp	al,0
#noport:
	pop	ax
	out	dx,al
	ret
_isport1 endp

;-----------------------------------------------------------------------------
; clrmode_standard - standard restore-old-video-mode.
;		     used for cards that just use int 10h ah=0, al=mode.
;-----------------------------------------------------------------------------

clrmode_standard proc near

	Int10	si
	ret

clrmode_standard endp

;-----------------------------------------------------------------------------
; setmode_standard - standard save-current-mode, set-new-mode.
;		     used for cards that just use int 10h ah=0, al=mode.
;   On return, sign flag set indicates an error.
;-----------------------------------------------------------------------------

setmode_standard proc near

	Int10	0F00h			; get current video mode
	movzx	ebp,al			; save it in ebp for return value.

	Int10	[esi].smi_mode		; set new video mode.

	Int10	0F00h			; get new current video mode,
	movzx	eax,al			; clean up returned mode value
	cmp	ax,[esi].smi_mode	; compare to requested mode,
	je	short #set_good 	; if equal, we're all set,
	mov	ebp,err_video_bios	; else indicate bios is unhappy.
	jmp	short #done

#set_good:
	call	wc_1x64k		; set up standard 1-window-of-64k stuff.
	call	_pj_vdrv_build_ytable	; set up ytable.
#done:
	mov	eax,ebp 		; return old mode.
	test	eax,eax 		; set flags for caller.
	ret

setmode_standard endp

;*****************************************************************************
;* Video 7 routines... (tweaked)
;*   These routines handle all models of V7VGA chips and cards except the
;*   VEGA chip/card (which doesn't have the extension registers we need).
;*****************************************************************************

;-----------------------------------------------------------------------------
; setbank_video7 - works for V7VGA chips revision 1 thru 3.
;-----------------------------------------------------------------------------

setbank_video7 proc near

	push	ecx			; save ecx to stack.
	mov	cl,al			; save bank number in ecx.
	mov	[edx].wrdcurbank,eax	; save the new bank number as the
	mov	[edx].wwrcurbank,eax	; current read and write banks.

	mov	dx,3C4h 		; set bank bit 0...
	mov	ah,cl
	and	ah,1
	mov	al,0F9h
	out	dx,ax

	mov	ah,cl			; set bank bit 1...
	and	ah,2
	shl	ah,4
	mov	dx,3CCh
	in	al,dx
	and	al,11011111b		; mask out existing bit
	mov	dx,3C2h
	or	al,ah			; put in new bit
	out	dx,al

	mov	dx,3C4h 		; set bank bits 3 & 4...
	mov	al,0F6h
	out	dx,al
	inc	dx
	in	al,dx
	and	al,11110000b		; mask out existing bank select bits
	and	cl,00001100b		; isolate new bits
	or	al,cl			; put new bits into read bank
	shr	cl,2			; move new bits over
	or	al,cl			; put new bits into write bank
	out	dx,al

	pop	ecx			; restore ecx.
	lea	edx,pj_vdrv_wcontrol	; restore edx.
	ret

setbank_video7 endp

;-----------------------------------------------------------------------------
; setbank_video7i - works for V7VGA chip revision 4 and up (i1024 card).
;-----------------------------------------------------------------------------

setbank_video7i proc near

	mov	[edx].wrdcurbank,eax	; save the new bank number as the
	mov	[edx].wwrcurbank,eax	; current read and write banks.

	mov	dx,3C4h
	shl	eax,12			; put bank # in hi-order nibble of ah.
	mov	al,0E8h 		; select single/write bank register
	out	dx,ax			; do it.

	lea	edx,pj_vdrv_wcontrol	; restore edx.
	ret

setbank_video7i endp

;-----------------------------------------------------------------------------
; clrmode_video7 - disable the extension registers, restore prior video mode.
;-----------------------------------------------------------------------------

clrmode_video7 proc near

	mov	dx,03C4h		; disable V7 extension registers...
	mov	ax,0AE06h		; load magic value: disable extensions.
	out	dx,ax			; do it.

	Int10	6F05h,si		; restore old video mode using V7 func.

	ret

clrmode_video7 endp

;-----------------------------------------------------------------------------
; setmode_video7 - set up video mode and enable extensions registers.
;-----------------------------------------------------------------------------

setmode_video7 proc near

	Int10	6F04h			; get current mode using V7 func.
	movzx	ebp,al			; save oldmode for retval.

	Int10	6F05h,[esi].smi_mode	; set new mode using V7 func.

	Int10	6F04h			; get new video mode using V7 func,
	movzx	eax,al			; compare it to the requested mode. if
	cmp	al,bptr [esi].smi_mode	; not equal, something failed, most
	je	short #set_good 	; likely a mode for which we don't
	mov	ebp,err_video_bios	; have enough onboard memory, but we
	jmp	short #done		; report err_video_bios to the caller.

#set_good:
	mov	dx,03C4h		; enable V7 extension registers, so
	mov	ax,0EA06h		; that we can do our bank switching.
	out	dx,ax			; (EA06h is a V7-defined magic value).

	call	wc_1x64k		; set up standard 1-window-of-64k stuff.
	call	_pj_vdrv_build_ytable	; set up ytable.
#done:
	mov	eax,ebp 		; return success/oldmode data.
	ret

setmode_video7 endp

;-----------------------------------------------------------------------------
; is_it_video7 - detect video7 cards by calling the V7 BIOS.
;-----------------------------------------------------------------------------

is_it_video7 proc near

	mov	dl,gs:[0487h]		; turn off V7 PURE VGA mode by turning
	or	dl,00010000b		; on the extensions-allowed bit in the
	mov	gs:[0487h],dl		; BIOS miscdata variable.

	push	edx			; this is just extreme paranoia.
	Int10	6F00h,0 		; Test for V7 using special V7 func.
	pop	edx			; restore edx.
	cmp	bx,'V7'                 ; Is the return value magical?
	jne	short #not_v7		; if not, punt.

	Int10	6F07h			; get memory & chip revision data. if
	cmp	bl,7Fh			; chip is > 7Fh it's a VEGA, which
	jbe	short #is_v7		; we can't deal with in this driver.

#not_v7:
	mov	gs:[0487h],dl		; restore the original BIOS miscdata
	xor	edi,edi 		; variable we tweaked, set retval to
	jmp	#done			; indicate not v7, and punt.

#is_v7:
	SetInfo 66h,640,400		; all V7 cards have this mode.
	mov	edi,1

	and	ah,7fh			; mask DRAM/VRAM bit (we don't care),
	cmp	ah,1			; remaining value is number of 256k
	jbe	short #check_chip	; banks; no more modes if 256k onboard.

	SetInfo 67h,640,480		; 512k+ cards have these modes too...
	SetInfo 68h,720,540
	SetInfo 69h,800,600
	add	edi,3

#check_chip:
	cmp	bl,50h			; check chip number, if it's not an
	jae	short #v7vram		; i1024, go set up regular V7VRAM.

	NameVec video7i 		; for i1024, device has own name and
	BankVec video7i 		; bankswitch vectors, but uses the
	ModeVec video7			; same set/clrmode vectors.
	jmp	short #done		; if i1024 has more modes; put em here.

#v7vram:
	NameVec video7			; we have vanilla v7vram chip...
	ModeVec video7			; set pointers to bankswitch routines
	BankVec video7			; for normal v7vga chips.

#done:
	mov	eax,edi
	ret

is_it_video7 endp

;*****************************************************************************
;* Tseng4000 routines... (tweaked)
;*****************************************************************************

;-----------------------------------------------------------------------------
; setbank_tseng4 - bankswitch routines for tseng4000.
;   note that tseng chips support separate read & write banks, and so do we,
;   since you get better performance (fewer bankswitches) with them.
;-----------------------------------------------------------------------------

setbank_rtseng4 proc near		; read bank for tseng4000

	mov	[edx].wrdcurbank,eax	; save the new read bank number.

	shl	eax,12			; move readbank bits to hi-order ah.
	mov	dx,3cdh
	in	al,dx			; get current bank register
	and	al,0Fh			; mask out old read bank bits
	or	al,ah			; lay in new bits
	out	dx,al			; store new bank register value.

	lea	edx,pj_vdrv_wcontrol
	ret

setbank_rtseng4 endp

setbank_wtseng4 proc near		; write bank for tseng4000

	mov	[edx].wwrcurbank,eax	; save the new write bank number.

	mov	ah,al			; move writebank bits to lo-order ah.
	and	ah,0Fh			; mask them to just 4 bits.
	mov	dx,3cdh
	in	al,dx			; get current bank register
	and	al,0F0h 		; mask out old write bank bits
	or	al,ah			; lay in new bits
	out	dx,al			; store new bank register value.

	lea	edx,pj_vdrv_wcontrol
	ret

setbank_wtseng4 endp

;-----------------------------------------------------------------------------
; setmode_tseng4
;-----------------------------------------------------------------------------

setmode_tseng4	proc near

	call	setmode_standard	; do standard setmode stuff.
	mov	ebp,eax 		; save retval in ebp.
	js	short #done		; punt on error.

	mov	dx,3bfh 		; Enable access to extended registers
	mov	al,3
	out	dx,al
	mov	dx,3d8h
	mov	al,0a0h
	out	dx,al
#done:
	mov	eax,ebp 		; return old mode.
	ret

setmode_tseng4	endp

;-----------------------------------------------------------------------------
; is_it_tseng4 - Detect tseng 4000
;-----------------------------------------------------------------------------

is_it_tseng4 proc near

	xor	edi,edi 		; assume not tseng 4000.

	mov	dx,3d4h 		; check for Tseng 4000 series
	mov	ax,0f33h
	call	_isport2
	jnz	#done

	mov	dx,3bfh 		; Enable access to extended registers
	mov	al,3
	out	dx,al
	mov	dx,3d8h
	mov	al,0a0h
	out	dx,al

	mov	dx,3cdh 		; test bank switch register
	mov	al,0ffh
	call	_isport1
	jnz	#done

	NameVec tseng4
	ModeVec tseng4,standard 	; special setmode, standard clrmode
	BankVec rtseng4,wtseng4

	SetInfo 2Dh,640,350
	SetInfo 2Fh,640,400
	add	edi,2

	mov	dx,3d4h 		;Tseng 4000 memory detect 1meg
	mov	al,37h
	out	dx,al
	inc	dx
	in	al,dx
	test	al,1000b		; if using 64kx4 RAMs then no more than
	jz	short #done		; 256k
	and	al,3
	cmp	al,1			; if 8 bit wide bus then only two
	jbe	short #done		; 256kx4 RAMs

	SetInfo 2Eh,640,480		; we have at least 512k
	SetInfo 30h,800,600
	add	edi,2

	cmp	al,2			; if 16 bit wide bus then four
	je	short #done		; 256kx4 RAMs, else
	SetInfo 38h,1024,768		; full meg with eight 256kx4 RAMs
	inc	edi
#done:
	mov	eax,edi
	ret

is_it_tseng4 endp

;*****************************************************************************
;* Tseng3000 routines... (tweaked)
;*****************************************************************************

;-----------------------------------------------------------------------------
; setbank_tseng3 - bankswitch routines for tseng3000.
;   note that tseng chips support separate read & write banks, and so do we,
;   since you get better performance (fewer bankswitches) with them.
;-----------------------------------------------------------------------------

setbank_rtseng3 proc near		 ; read bank for tseng3000

	mov	[edx].wrdcurbank,eax	; save the new bank number.

	mov	ah,al			; save new bank # in ah
	and	ah,7			; mask to 3 bits,
	shl	ah,3			; shift bits up to readbank position.
	mov	dx,3cdh
	in	al,dx			; get current bank register
	and	al,11000111b		; mask out old read bank
	or	al,ah			; lay in new read bank bits
	out	dx,al			; store new bank reg value.

	lea	edx,pj_vdrv_wcontrol
	ret

setbank_rtseng3 endp

setbank_wtseng3 proc near		 ; write bank for tseng3000

	mov	[edx].wwrcurbank,eax	; save the new bank number.

	mov	ah,al			; save new bank # in ah
	and	ah,7			; mask to 3 bits
	mov	dx,3cdh
	in	al,dx			; get current bank register
	and	al,11111000b		; mask out old write bank
	or	al,ah			; lay in new write bank bits
	out	dx,al			; store new bank reg value.

	lea	edx,pj_vdrv_wcontrol
	ret

setbank_wtseng3 endp

;-----------------------------------------------------------------------------
; is_it_tseng3 - detect tseng 3000
;-----------------------------------------------------------------------------

is_it_tseng3 proc near

	xor	edi,edi 		; assume not tseng 3000

	mov	dx,3d4h 		; Test for Tseng 3000
	mov	ax,1f25h		; is the Overflow High register there?
	call	_isport2
	jnz	#done

	mov	al,03fh 		;bottom six bits only
	mov	dx,3cdh 		;test bank switch register
	call	_isport1
	jnz	#done

	NameVec tseng3
	ModeVec standard
	BankVec rtseng3,wtseng3

	SetInfo 2Dh,640,350
	SetInfo 2Eh,640,480
	SetInfo 2Fh,720,512
	SetInfo 30h,800,600
	add	edi,4
#done:
	mov	eax,edi
	ret

is_it_tseng3 endp


;*****************************************************************************
;* Trident... (tweaked)
;*   These routines handle Trident 8800 and 8900 chips.  The setbank routines
;*   are shared by the Everex EVGA512 hardware, which is Trident-based.
;*****************************************************************************

;-----------------------------------------------------------------------------
; setbank_trident
;-----------------------------------------------------------------------------

setbank_trident proc near

	mov	[edx].wrdcurbank,eax	; save the new bank number as the
	mov	[edx].wwrcurbank,eax	; current read and write banks.

	mov	ah,al
	xor	ah,00000010b		; invert PAGE bit
	mov	dx,3c4h
	mov	al,0eh
	out	dx,ax

	lea	edx,pj_vdrv_wcontrol
	ret

setbank_trident endp

;-----------------------------------------------------------------------------
; clrmode_trident
;-----------------------------------------------------------------------------

clrmode_trident proc near

	mov	dx,3c4h 		; ensure that the old/new mode register
	mov	al,0bh			; definition is left in the oldmode
	out	dx,al			; state... 0Bh selects the chip version
	inc	dl			; register. writing to this register
	out	dx,al			; forces the toggle state to oldmode.

	call	clrmode_standard
	ret

clrmode_trident endp

;-----------------------------------------------------------------------------
; setmode_trident
;-----------------------------------------------------------------------------

setmode_trident proc near

	call	setmode_standard	; set up video mode.
	mov	ebp,eax 		; save return value/status.
	js	short #done		; punt on error.

#enable_ext:
	mov	dx,3c4h 		; ensure that the old/new mode register
	mov	al,0bh			; definition is toggled to the newmode
	out	dx,al			; state... 0Bh selects the chip version
	inc	dl			; register. writing to this register
	out	dx,al			; forces the toggle state to oldmode;
	in	al,dx			; reading then toggles state to newmode.

	mov	dx,3ceh 		; set page size to 64k...
	mov	al,6
	out	dx,al
	inc	dl
	in	al,dx
	dec	dl
	or	al,4
	mov	ah,al
	mov	al,6
	out	dx,ax

#done:
	mov	eax,ebp 		; return old mode.
	ret

setmode_trident endp

;-----------------------------------------------------------------------------
; is_it_trident
;-----------------------------------------------------------------------------

is_it_trident proc near

	xor	edi,edi 		; assume it's not a trident

	mov	dx,3c4h 		; detect trident by checking for
	mov	al,0eh			; an inverting bit in reg 0Eh.
	out	dx,al
	inc	dx
	in	al,dx			; read current value
	mov	ah,al			; save it
	mov	al,0			; write a 0
	out	dx,al
	in	al,dx			; read it back
	xchg	ah,al			; save it, restore original to al
	out	dx,al			; restore original to reg 0Eh.

	and	ah,0fh			; mask high bits we don't care about
	cmp	ah,02h			; if the 0000b we wrote read back as
	jne	#done			; 0010b it's a trident.

	mov	dx,3c4h
	mov	al,0bh			; select chip version register
	out	dx,al
	inc	dl
	in	al,dx			; read version twice because each read
	in	al,dx			; toggles old/new mode state, and we
	cmp	al,0fh			; want to leave it the way we found it.
	ja	#done
	cmp	al,2
	jb	#done

	NameVec trident 		; set device name
	ModeVec trident 		; set mode pointers
	BankVec trident 		; set bankswitch pointers

	SetInfo 5Ch,640,400		; all trident chips have these
	SetInfo 5Dh,640,480		; two modes.
	add	edi,2

	cmp	al,2			; if chip version register value is < 3,
	jbe	short #done		; we have 8800 chip, with only 2 modes.

	mov	dx,3d5h 		; get the memory configuration
	mov	al,1fh			; register to see if there's
	out	dx,al			; enough memory to support the
	inc	dx			; higher resolutions...
	in	al,dx
	and	al,3

	cmp	al,1
	jb	short #done
	SetInfo 5Eh,800,600		; if we have 512k or more, we can
	inc	edi			; do this mode.

	cmp	al,1			; if we have 512k that's it, if we
	je	short #done		; have more than 512k we can also
	SetInfo 62h,1024,768		; do this mode..
	inc	edi
#done:
	mov	eax,edi
	ret

is_it_trident endp

;*****************************************************************************
;* Everex routines...(tweaked)
;*  These routines support both the original and new (Trident-based) Everex
;*  hardware.
;*****************************************************************************

;-----------------------------------------------------------------------------
; setbank_everex
;   This routine used for older Everex hardware.  The newer EVGA512 uses
;   the trident chipset, and thus the trident bankswitch routines.
;-----------------------------------------------------------------------------

setbank_everex proc near

	mov	[edx].wrdcurbank,eax	; save the new bank number as the
	mov	[edx].wwrcurbank,eax	; current read and write banks.

	push	ecx			; this code straight from vgakit.
	mov	cl,al
	mov	dx,3c4h
	mov	al,8
	out	dx,al
	inc	dl
	in	al,dx
	dec	dl
	shl	al,1
	shr	cl,1
	rcr	al,1
	mov	ah,al
	mov	al,8
	out	dx,ax
	mov	dx,3cch
	in	al,dx
	mov	dx,3c2h
	and	al,0dfh
	shr	cl,1
	jc	short #nob2
	or	al,20h
#nob2:
	out	dx,al

	pop	ecx
	lea	edx,pj_vdrv_wcontrol
	ret

setbank_everex endp

;-----------------------------------------------------------------------------
; setmode_everex - Set mode for Everex.
;-----------------------------------------------------------------------------

setmode_everex proc near

	Int10	0F00h			; get current video mode
	movzx	ebp,al			; save it in ebp for return value.

	Int10	0070h,[esi].smi_mode	; set new video mode

	;Int10	0F00h			; get new video mode,
	;movzx	eax,al			; clean up mode value,
	;cmp	ax,[esi].smi_mode	; compare to requested mode,
	;je	short #set_good 	; if equal, we're all set,
	;mov	ebp,err_video_bios	; else indicate bios is unhappy.
	;jmp	short #done
#set_good:
	call	wc_1x64k		; set up standard 1-window-of-64k stuff.
	call	_pj_vdrv_build_ytable	; set up ytable.
#done:
	mov	eax,ebp 		; return old mode.
	ret

setmode_everex endp

;-----------------------------------------------------------------------------
; clrmode_evga512
;-----------------------------------------------------------------------------

clrmode_evga512 proc near

	mov	dx,3c4h 		; ensure that the old/new mode register
	mov	al,0bh			; definition is left in the oldmode
	out	dx,al			; state... 0Bh selects the chip version
	inc	dl			; register. writing to this register
	out	dx,al			; forces the toggle state to oldmode.
#notevga:
	call	clrmode_standard
	ret

clrmode_evga512 endp

;-----------------------------------------------------------------------------
; setmode_evga512 - Set mode for EVGA512
;-----------------------------------------------------------------------------

setmode_evga512 proc near

	call	setmode_everex		; call basic setmode
	test	eax,eax 		; see if it worked
	js	short #done		; if not, punt

	mov	ebp,eax 		; save original return code

; NOTE: The following code totally confuses me -- I copied it directly
;	from the EVGA512 driver Jake wrote/tweaked for us.

	Mov	DX, 03C4H		; FOR EVEREX - Fix SR2
	Mov	AX, 0F02H
	Out	DX, AX
;
; Now we need to make sure that our palette is set correctly. This means
; that the first-stage EGA to VGA LUT must be set properly for all other
; VGA palette operations to work properly.
;
	mov	DX, 3DAH
	in	AL, DX			; Clear flip-flop register state.

	mov	ECX, 10H		; Number of EGA entries.
	xor	AL, AL
	mov	DX, 3C0H		; Point to the EGA regs.
EGAPalLoop:
	out	DX, AL			; Set the index.
	jmp	$+2			; Wait for it to be ready again
	out	DX, AL			; Set the data
	inc	AL			; We want a one-to-one ramp.
	loop	EGAPalLoop

	mov	AL, 20H
	out	DX, AL			; We're done - tell VGA.

; end of mystery code.

	mov	eax,ebp 		; return old mode
#done:
	ret

setmode_evga512 endp

;-----------------------------------------------------------------------------
; Everex detect - detect Everex card via call to its BIOS.
;-----------------------------------------------------------------------------

is_it_everex proc near

	xor	edi,edi 		; assume not everex

	Int10	7000h,0 		; check for everex by calling special
	cmp	al,70h			; bios function, on return, check for
	jne	#done			; magic value.	if not there, punt.

	NameVec everex
	ModeVec everex,standard
	BankVec everex

	SetInfo 13h,640,350
	SetInfo 14h,640,400
	SetInfo 15h,512,480
	add	edi,3

	and	ch,11000000b
	jz	short #not512

	SetInfo 30h,640,480
	SetInfo 31h,800,600
	add	edi,2
#not512:
	and	dx,0fff0h
	cmp	dx,6780h
	je	short #trident_chip
	cmp	dx,2360h
	jnz	short #done
#trident_chip:
	NameVec evga512
	ModeVec evga512
	BankVec trident
#done:
	mov	eax,edi
	ret

is_it_everex endp

;*****************************************************************************
;* Chips & Tech...(tweaked)
;*****************************************************************************

;-----------------------------------------------------------------------------
; setbank_chips451 - handle 451/455/456 models of the chip
;-----------------------------------------------------------------------------

setbank_chips451  proc near

	mov	[edx].wrdcurbank,eax	; save the new bank number as the
	mov	[edx].wwrcurbank,eax	; current read and write banks.

	mov	ah,al			; move bank # to ah.
	mov	al,0bh			; bank reg on 541 chip is extreg 0Bh
	mov	dx,3d6h
	out	dx,ax

	lea	edx,pj_vdrv_wcontrol
	ret

setbank_chips451  endp

;-----------------------------------------------------------------------------
; setbank_chips452 - handle 452/453 models of the chip
;-----------------------------------------------------------------------------

setbank_chips452  proc near

	mov	[edx].wrdcurbank,eax	; save the new bank number as the
	mov	[edx].wwrcurbank,eax	; current read and write banks.

	shl	eax,10			; change 64k bank # into 16k bank in ah.
	mov	al,10h			; bank reg on 452 chips is extreg 10h.
	mov	dx,3d6h
	out	dx,ax

	lea	edx,pj_vdrv_wcontrol
	ret

setbank_chips452  endp

;-----------------------------------------------------------------------------
; clrmode_chipstech
;-----------------------------------------------------------------------------

clrmode_chipstech proc near

	mov	dx,chipdata		;place chip in setup mode
	mov	al,1eh
	out	dx,al
	mov	dx,103h 		;disable extended registers
	xor	al,al
	out	dx,al
	mov	dx,chipdata		;bring chip out of setup mode
	mov	al,0eh
	out	dx,al

	call	clrmode_standard
	ret

clrmode_chipstech endp

;-----------------------------------------------------------------------------
; setmode_chipstech
;-----------------------------------------------------------------------------

setmode_chipstech proc near

	call	setmode_standard	; set video mode
	mov	ebp,eax 		; save return value
	js	short #done		; punt on error.

	mov	dx,chipdata		; place chip in setup mode
	mov	al,1eh
	out	dx,al
	mov	dx,103h 		; enable extended registers
	mov	al,80h
	out	dx,al
	mov	dx,chipdata		; bring chip out of setup mode
	mov	al,0eh
	out	dx,al

#done:
	mov	eax,ebp
	ret

setmode_chipstech endp

;-----------------------------------------------------------------------------
; Chips and Tech detect - detect chipstech hardware via call to its BIOS.
;-----------------------------------------------------------------------------

is_it_chipstech proc near

	xor	edi,edi 		; assume not chipstech

	Int10	5F00h,0 		; check for chipstech via call to
	cmp	al,5fh			; special bios function.  if magic
	jne	#done			; value not present, punt.

	NameVec chipstech
	ModeVec chipstech		; set pointers to set/clrmode routines.
	BankVec chips452		; assume model 452/453 bank switching
	shr	bl,4			; move model number to low-order bl.
	test	bl,1			; if bit zero set, or value is
	jnz	short #is_452		; greater than two, it must be a
	cmp	bl,2			; model 452/453 chip, and we're
	jae	short #is_452		; already set for that, else set
	BankVec chips451		; model 451/455/456 bank switching.
#is_452:
	test	cl,2			; test 8-bit DAC flag,
	setnz	dac8bit 		; set our flag accordingly.

	mov	ax,46E8h		; port address for PC/AT systems.
	test	cl,1			; are we on a PC/AT or a PS/2?
	jz	short #is_pc		; yep, we're all set.
	mov	ax,3C3h 		; nope, set port for PS/2 machines
#is_pc:
	mov	chipdata,ax		; save port for set/clrmode processing.

	SetInfo 78h,640,400		; all models have this mode.
	inc	edi

	cmp	bh,1			; if 512k or more onboard, we have
	jb	short #done		; additional modes, else we're done.
	SetInfo 79h,640,480
	SetInfo 7Ah,768,576
	SetInfo 7Ch,800,600		; ??? vgakit says 7Bh, book says 7Ch
	add	edi,3

	cmp	bh,2			; if 1m or more onboard, we have
	jb	short #done		; additional modes, else we're done.
	SetInfo 7Eh,1024,768
	inc	edi
#done:
	mov	eax,edi
	ret

is_it_chipstech endp

;*****************************************************************************
;* ATI VGA routines...(tweaked)
;*  These routines support all flavors of ATI.	The 18800-1 and 28800 chips
;*  have the capability for separate read & write bankswitching, but right
;*  now we don't use that or even differentiate between chips.
;*****************************************************************************

;-----------------------------------------------------------------------------
; ATI VGA
;-----------------------------------------------------------------------------

setbank_ativga proc near

	mov	[edx].wrdcurbank,eax	; save the new bank number as the
	mov	[edx].wwrcurbank,eax	; current read and write banks.

	mov	dx,chipdata		; safe way of getting extreg port #
	mov	ah,al			; save new bank number for a mo..
	mov	al,0b2h 		; index to bank select register.
	out	dx,al
	inc	dl
	in	al,dx			; get current banksel value.
	dec	dl
	shl	ah,1
	and	al,11100001b		; mask out old banksel bits.
	or	ah,al			; lay in new bank number.
	mov	al,0b2h 		; reselect bank select register
	out	dx,ax			; set new bank.

	lea	edx,pj_vdrv_wcontrol
	ret

setbank_ativga endp

;-----------------------------------------------------------------------------
; ATI VGA detect...
;  The ATI cards are ID'd by a nine-byte string at C000:0031; if the string
;  is '761295520' it is some flavor of ATI card.
;-----------------------------------------------------------------------------

is_it_ati proc near

	xor	edi,edi 		; assume not ati

	cmp	dptr gs:[0C0031h],'2167'; first piece of key, in dword order
	jne	#done
	cmp	dptr gs:[0C0035h],'2559'; second piece of key, in dword order
	jne	#done
	cmp	bptr gs:[0C0039h],'0'   ; last piece of key.
	jne	#done

	NameVec ativga
	ModeVec standard		; set pointers to set/clrmode routines.
	BankVec ativga,ativga		; set pointers to bankswitch routines.

	SetInfo 61h,600,400		; All ATI chips have this mode.
	inc	edi

	mov	dx,wptr gs:[0C0010h]	; get port # for extended register
	mov	chipdata,dx		; save for bankswitching routines.

	mov	bl,bptr gs:[0C0043h]	; get chip version number
	cmp	bl,'3'                  ; Use different method to determine
	jae	short #v6up		; memory size of chip v3 or higher

	mov	al,0bbh
	out	dx,al
	inc	dx
	in	al,dx			; Get ramsize byte for chip v1 & v2
	test	al,20h
	jz	short #done
	jmp	short #has512

#v6up:
	mov	al,0b0h 		; Method used for newer ATI chip
	out	dx,al			; versions
	inc	dx
	in	al,dx			; Get ramsize byte for versions 3-5
	test	al,10h			; Check if ramsize byte indicates
	jnz	short #has512		; 256K or 512K bytes

	cmp	bl,'4'                  ; Check for ramsize for ATI chip
	jb	short #done		; versions 4 & 5
	test	al,8			; Check if version 5 ATI chip has 1024K
	jz	short #done
#has512:
	SetInfo 62h,640,480
	SetInfo 63h,800,600
	add	edi,2
#done:
	mov	eax,edi
	ret

is_it_ati endp

;*****************************************************************************
;* Ahead System Versions A and B...(tweaked)
;*  This chipset/card is not in the svga book, so all this is from vgakit.
;*****************************************************************************

;-----------------------------------------------------------------------------
; Setbank Ahead Systems Version A
;-----------------------------------------------------------------------------

setbank_aheada proc near		       ;Ahead Systems Ver A

	mov	[edx].wrdcurbank,eax	; save the new bank number as the
	mov	[edx].wwrcurbank,eax	; current read and write banks.

	mov	ah,al
	and	ah,0Fh

	mov	dx,3cch 		;bit 0
	in	al,dx
	and	al,11011111b
	shr	ah,1
	jnc	short #skpa
	or	al,00100000b
#skpa:
	mov	dx,3c2h
	out	dx,al			; save bit 0

	mov	dx,3cfh 		;bits 1,2,3
	mov	al,0
	out	dx,al
	inc	dx
	in	al,dx
	dec	dx
	and	al,11111000b
	or	ah,al
	mov	al,0
	out	dx,ax

	lea	edx,pj_vdrv_wcontrol
	ret

setbank_aheada endp

;-----------------------------------------------------------------------------
; Setbank Ahead Systems Version B
;-----------------------------------------------------------------------------

setbank_aheadb proc near

	mov	[edx].wrdcurbank,eax	; save the new bank number as the
	mov	[edx].wwrcurbank,eax	; current read and write banks.

	mov	dx,3CEh
	mov	ah,al			; set one of the banks (read or write,
	shl	al,4			; I have no idea which is which).
	or	ah,al			; set the other bank to match
	mov	al,0dh
	out	dx,ax

	lea	edx,pj_vdrv_wcontrol
	ret

setbank_aheadb endp

;-----------------------------------------------------------------------------
; setmode_aheada
;-----------------------------------------------------------------------------

setmode_ahead proc near

	call	setmode_standard	; set up video mode
	mov	ebp,eax 		; save return value
	js	short #done		; punt on error

#set_good:
	mov	dx,3ceh 		; Enable extended registers
	mov	ax,200fh
	out	dx,ax

#done:
	mov	eax,ebp
	ret

setmode_ahead endp

;-----------------------------------------------------------------------------
; Detect Ahead A or B detect...
;  Another one with a shakey-looking detect routine.
;-----------------------------------------------------------------------------

is_it_ahead proc near			; naw, it's afoot

	xor	edi,edi

	mov	dx,3ceh 		; I have no idea what this is doing...
	mov	ax,200fh
	out	dx,ax
	inc	dx
	jmp	short #delay
#delay:
	in	al,dx
	cmp	al,21h
	je	short #ver_b
	cmp	al,20h
	jne	#done

	NameVec aheada
	ModeVec ahead,standard
	BankVec aheada
	jmp	short #setinfos
#ver_b:
	NameVec aheadb
	ModeVec ahead,standard
	BankVec aheadb
#setinfos:
	SetInfo 60h,640,400
	SetInfo 61h,640,480
	SetInfo 62h,800,600
	add	edi,3
#done:
	mov	eax,edi
	ret

is_it_ahead endp

;*****************************************************************************
;* Oak Tech OTI-067...(tweaked)
;*  Another one that's not in the svga book.
;*****************************************************************************

;-----------------------------------------------------------------------------
; Setbank oaktech
;-----------------------------------------------------------------------------

setbank_oaktech proc near		       ;Oak Technology Inc OTI-067

	mov	[edx].wrdcurbank,eax	; save the new bank number as the
	mov	[edx].wwrcurbank,eax	; current read and write banks.

	and	al,0Fh
	mov	ah,al
	shl	al,4
	or	al,ah
	mov	dx,3dfh
	out	dx,al

	lea	edx,pj_vdrv_wcontrol
	ret

setbank_oaktech endp

;-----------------------------------------------------------------------------
; Detect oaktech
;-----------------------------------------------------------------------------

is_it_oaktech proc near

	xor	edi,edi

	mov	dx,3deh 		;Test for Oak Technology
	mov	ax,0ff11h		;look for bank switch register
	call	_isport2
	jne	short #done

	NameVec oaktech
	ModeVec standard
	BankVec oaktech
	SetInfo 53h,640,480
	inc	edi

	mov	al,0dh
	out	dx,al
	inc	dx
	jmp	short #delay
#delay:
	in	al,dx
	test	al,11000000b
	jz	short #done
	SetInfo 54h,800,600
	inc	edi
#done:
	mov	eax,edi
	ret

is_it_oaktech endp

;*****************************************************************************
;* Paradise...
;*****************************************************************************

;-----------------------------------------------------------------------------
; setbank_paradise - bankswitching for PVGA1A, WD90C00, WD90C10, and for
;    the read bank on the WD90C11.
;-----------------------------------------------------------------------------

setbank_paradise proc near		       ;Paradise

	mov	[edx].wrdcurbank,eax	; save the new bank number as the
	mov	[edx].wwrcurbank,eax	; current read and write banks.


	mov	dx,3CEh
	shl	ax,12			; put bank number into hi-order ah..
	mov	al,9
	out	dx,ax

	lea	edx,pj_vdrv_wcontrol
	ret

setbank_paradise endp

;-----------------------------------------------------------------------------
; setbank_wparadise - bankswitching for the write bank on a WD90C11.
;-----------------------------------------------------------------------------

setbank_wparadise proc near			;Paradise

	mov	[edx].wrdcurbank,eax	; save the new bank number as the
	mov	[edx].wwrcurbank,eax	; current read and write banks.


	mov	dx,3CEh
	shl	ax,12			; put bank number into hi-order ah..
	mov	al,10
	out	dx,ax

	lea	edx,pj_vdrv_wcontrol
	ret

setbank_wparadise endp

;-----------------------------------------------------------------------------
; xon/xoff_paradise - enable/disable paradise extension regs
;-----------------------------------------------------------------------------

xon_paradise proc near
	mov	dx,3CEh
	mov	ax,050fh
	out	dx,ax			; enable PR1-PR9 regs
	mov	dx,3D4h
	mov	ax,8529h
	out	dx,ax			; enable PR10-14 regs
	mov	dx,3C4h
	mov	ax,4806h
	out	dx,ax			; enable extended CRTC regs
	ret
xon_paradise endp

xoff_paradise proc near
	mov	dx,3CEh
	mov	ax,000fh
	out	dx,ax			; disable PR1-PR9 regs
	mov	dx,3D4h
	mov	ax,0029h
	out	dx,ax			; disable PR10-14 regs
	mov	dx,3C4h
	mov	ax,0006h
	out	dx,ax			; disable extended CRTC regs
	ret
xoff_paradise endp

;-----------------------------------------------------------------------------
; clrmode_paradise
;-----------------------------------------------------------------------------

clrmode_paradise proc near

	cmp	chipdata,0		; do we have the WD90C11 chip?
	je	short #notc11		; nope, skip the bank junk.

	mov	dx,3C4h 		; disable separate read/write bank
	mov	al,11h			; switching on the C11 chip...
	out	dx,al
	inc	dx
	in	al,dx
	and	al,01111111b
	out	dx,al
	mov	dx,3CEh
	mov	al,0bh
	out	dx,al
	inc	dx
	in	al,dx
	and	al,11110111b
	out	dx,al
#notc11:
	call	xoff_paradise		; disable ext regs.
	call	clrmode_standard
	ret

clrmode_paradise endp

;-----------------------------------------------------------------------------
; setmode_paradise
;-----------------------------------------------------------------------------

setmode_paradise proc near

	call	setmode_standard	; set up video mode
	mov	ebp,eax 		; save return value
	js	short #done		; punt on error

	call	xon_paradise		; enable extension regs

	cmp	chipdata,0		; do we have a WD90C11 chip?
	je	short #done		; nope, all done.

	mov	dx,3C4h 		; enable separate read/write bank
	mov	al,11h			; switching on the C11 chip...
	out	dx,al
	inc	dx
	in	al,dx
	or	al,10000000b
	out	dx,al
	mov	dx,3CEh
	mov	al,0bh
	out	dx,al
	inc	dx
	in	al,dx
	or	al,00001000b
	out	dx,al

#done:
	mov	eax,ebp 		; return old mode.
	ret

setmode_paradise endp

;-----------------------------------------------------------------------------
; Paradise detect...
;  Paradise cards are ID'd by a 4-byte string with the value 'VGA=' at
;  location C000:007D.	If this test passes, we follow up with a call
;  to a Paradise-specific BIOS function as a double-check.
;-----------------------------------------------------------------------------

is_it_paradise proc near

	xor	edi,edi 		; Assume not paradise

	cmp	dptr gs:[0C007Dh],'=AGV'; Paradise key, in dword order.
	jne	#done			; if not equal, definately not Paradise.

	Int10	007Fh,0200h		; On return, bh=magic,ch=# of 64k banks.
	cmp	bh,7Fh			; did we get the magic number back?
	jne	#done			; nope, must not be Paradise.

	NameVec paradise		; It's some sort of paradise, set
	ModeVec paradise		; up the name, mode, and
	BankVec paradise		; bankswitch vectors.

	SetInfo 5Eh,640,400		; all paradise cards have this mode.
	inc	edi
	cmp	ch,8			; if we have 8 or more 64k banks,
	jl	short #not512k		; we also have a 640x480x256 mode.
	SetInfo 5Fh,640,480
	inc	edi

#not512k:

	mov	chipdata,0		; signal non-WD90C11 chip model.

	call	xon_paradise		; Enable extregs while determining chip.

	mov	dx,3D4h 		; determine if it's a PVGA1A chip,
	mov	al,2Bh			; by writing a value to the scratchpad
	out	dx,al			; register (PR12) and then reading it
	inc	dx			; back to see if the write worked.
	in	al,dx			; (we preserve the value and restore
	mov	bl,al			; it after, in case the write does
	mov	al,0AAh 		; work).  if the write didn't work,
	out	dx,al			; there is no scratchpad reg, and
	nop				; thus the chip is a PVGA1A rather
	in	al,dx			; than one of the WD90Cxx models.
	mov	bh,al
	mov	al,bl
	out	dx,al
	cmp	bh,0AAh
	jne	#pvga1a

	mov	dx,3C4h 		; we know it's a WD90Cxx chip of some
	mov	al,12h			; sort, now figure out which.  first,
	out	dx,al			; we distinguish between the C00
	inc	dx			; model and the C10/C11 models by
	in	al,dx			; writing a 0 then a 1 to bit 6 of the
	mov	bl,al			; miscellaneous control register.
	and	al,10111111b		; if the 0 reads back as 0 and the
	out	dx,al			; 1 reads back as 1, then we have
	nop				; a C10/C11 model chip, and we restore
	in	al,dx			; the original value to the register.
	test	al,01000000b		; if the readbacks don't come out
	jnz	#wd90c00		; the same as what we wrote, we have
	mov	al,bl			; the C00 model, and since the writes
	or	al,01000000b		; weren't working, there's no need
	out	dx,al			; to restore the original value.
	nop
	in	al,dx
	test	al,01000000b
	jz	#wd90c00
	mov	al,bl
	out	dx,al

	mov	dx,3C4h 		; we know it's a WD90C10 or WD90C11
	mov	al,10h			; chip, now figure out which.
	out	dx,al			; this uses the same concept as the
	inc	dx			; prior test, except this time we
	in	al,dx			; test bit 3 of the extended register
	mov	bl,al			; at index 10h (which my book doesn't
	and	al,11111011b		; name or describe).
	out	dx,al
	nop
	in	al,dx
	test	al,00000100b
	jnz	#wd90c10
	mov	al,bl
	or	al,00000100b
	out	dx,al
	nop
	in	al,dx
	test	al,00000100b
	jz	#wd90c10
	mov	al,bl
	out	dx,al

	mov	chipdata,1		; signal that we have a WD90C11 chip.
	BankVec paradise,wparadise	; special bankswitching for this chip.

	cmp	ch,8			; if we also have 512k or more, we
	jl	short #not512kc11	; have another video mode on this chip.
	SetInfo 5Ch,800,600
	inc	edi

#not512kc11:
#wd90c10:
#wd90c00:
#pvga1a:
	call	xoff_paradise
#done:
	mov	eax,edi
	ret

is_it_paradise endp

;*****************************************************************************
;* GENOA routines...
;*****************************************************************************

;-----------------------------------------------------------------------------
; setbank_genoa
;-----------------------------------------------------------------------------

setbank_genoa proc near

	mov	[edx].wrdcurbank,eax	; save the new bank number as the
	mov	[edx].wwrcurbank,eax	; current read and write banks.

	mov	ah,al			; straight from vgakit 5.0...
	shl	al,3
	or	ah,al
	mov	al,6
	or	ah,40h
	mov	dx,3c4h
	out	dx,ax

	lea	edx,pj_vdrv_wcontrol
	ret

setbank_genoa endp

;-----------------------------------------------------------------------------
; is_it_genoa
;  early genoa cards were based on tseng 3000; this detect should find only
;  later (non-tseng) cards; the normal tseng3 routines handle early genoa.
;-----------------------------------------------------------------------------

is_it_genoa proc near

	xor	edi,edi 		; assume not genoa

	mov	dx,3d4h
	mov	ax,032eh		; check for Herchi Register
	call	_isport2
	jnz	#done
	mov	dx,3c4h 		; check for memory segment register
	mov	ax,3f06h
	call	_isport2
	jnz	#done

	NameVec genoa
	ModeVec standard
	BankVec genoa

	SetInfo 2Dh,640,350
	SetInfo 2Eh,640,480
	SetInfo 2Fh,720,512
	SetInfo 30h,800,600
	add	edi,4
#done:
	mov	eax,edi
	ret

is_it_genoa endp

;*****************************************************************************
;* Compaq routines
;*****************************************************************************

;-----------------------------------------------------------------------------
; setbank_compaq
;-----------------------------------------------------------------------------

setbank_compaq proc near

	mov	[edx].wrdcurbank,eax	; save the new bank number as the
	mov	[edx].wwrcurbank,eax	; current read and write banks.

	push	eax
	mov	dx,3ceh
	mov	ax,50fh 		; unlock extended registers
	out	dx,ax
	pop	eax
	mov	ah,al
	shl	ah,4			; convert 64k bank to 4k window number
	mov	al,45h
	out	dx,ax

	lea	edx,pj_vdrv_wcontrol
	ret

setbank_compaq endp

;-----------------------------------------------------------------------------
; is_it_compaq
;-----------------------------------------------------------------------------

is_it_compaq proc near

	xor	edi,edi 		; assume not compaq

	Int10	0BF03h,0,0
	cmp	ax,0BF03h
	jne	short #done

	test	cl,40h			; is 640x480x256 available?
	jz	short #done

	NameVec compaq
	ModeVec standard
	BankVec compaq

	SetInfo 2Eh,640,480
	inc	edi
#done:
	mov	eax,edi
	ret

is_it_compaq endp

;*****************************************************************************
;* NCR routines
;*****************************************************************************

;-----------------------------------------------------------------------------
; setbank_ncr
;-----------------------------------------------------------------------------

setbank_ncr proc near

	mov	[edx].wrdcurbank,eax	; save the new bank number as the
	mov	[edx].wwrcurbank,eax	; current read and write banks.

	mov	dx,3c4h
	mov	ah,al
	shl	ah,2			; convert 64k bank to 16k window number
	mov	al,18h
	out	dx,ax
	mov	ax,19h			; why do I have the odd feeling this should be al, not ax???
	out	dx,ax

	lea	edx,pj_vdrv_wcontrol
	ret

setbank_ncr endp

;-----------------------------------------------------------------------------
; is_it_ncr
;-----------------------------------------------------------------------------

is_it_ncr proc near

	xor	edi,edi 		; assume not ncr

	mov	dx,3c4h 		; Test for NCR 77C22E
	mov	ax,0ff05h
	call	_isport2
	jnz	#done
	mov	ax,5			; Disable extended registers
	out	dx,ax
	mov	ax,0ff10h		; Try to write to extended register 10
	call	_isport2		; If it writes then not NCR
	jz	#done
	mov	ax,105h 		; Enable extended registers
	out	dx,ax
	mov	ax,0ff10h
	call	_isport2
	jnz	#done			; If it does NOT write then not NCR

	NameVec ncr
	ModeVec standard
	BankVec ncr

	SetInfo 5Eh,640,400
	SetInfo 5Fh,640,480
	SetInfo 5Ch,800,600
	add	edi,3

#done:
	mov	eax,edi
	ret

is_it_ncr endp

;*****************************************************************************
;* VESA BIOS Routines...
;*****************************************************************************

;-----------------------------------------------------------------------------
; VESA Detect
;-----------------------------------------------------------------------------

is_it_vesa proc near

	call	pj_vesa_detect
	test	eax,eax
	jz	short #done

	mov	vsetmode,offset pj_vesa_setmode
	mov	vclrmode,offset pj_vesa_clrmode
	mov	vcleanup,offset pj_vesa_free_dosbuf
	mov	vdevname,offset vname_vesa

#done:
	ret

is_it_vesa endp

;*****************************************************************************
;* Errcode pj_svga_init(SMInfo *psmodes)
;*   Try to figure out what video card we're dealing with.  If we find a card
;*   we recognize, the mode info array will be filled in with modes we can
;*   handle, and the return value will be the number of modes we put in the
;*   array.  If we can't figure out what card we're dealing with, we return
;*   Err_no_display.
;*****************************************************************************

pj_svga_init proc near

	Entry
	Args	#psmodes
	Save	ebx,esi,edi,ebp
	mov	esi,#psmodes

;-----------------------------------------------------------------------------
; first we'll do the 'safe' tests...those cards which can be detected by a
; call to the video BIOS or a search in the BIOS ROM for a signature string.
;-----------------------------------------------------------------------------

      if VESA_FIRST
	TestFor vesa
      endif

	TestFor video7
	TestFor chipstech
	TestFor ati
	TestFor paradise
	TestFor everex
	TestFor trident
	TestFor compaq

;-----------------------------------------------------------------------------
; now we do the shakey tests, those requiring writing to hardware registers.
;   note that these tests are still done in the same order as they were in
;   vgakit, except that a few were moved to the safe-test list, above. as
;   near as I can tell, the order may be critical, based on different cards
;   responding differently to the same registers during detection.
;-----------------------------------------------------------------------------

	TestFor tseng4			; checks tseng4000.
	TestFor tseng3			; checks tseng3000.
	TestFor genoa			; another that prolly s/b after tseng
	TestFor ahead			; checks aheadA & aheadB.
	TestFor oaktech
	TestFor ncr

;-----------------------------------------------------------------------------
; we can't find specific hardware, see if there's a VESA BIOS installed...
; this is tested for last because it's the lowest-performance alternative.
; (per Jim's request, vesa can be the first thing tested for instead of
;  the last)
;-----------------------------------------------------------------------------

      if NOT VESA_FIRST
	TestFor vesa
      endif

	mov	eax,err_no_display	; if we made it here, we can't do it.
#done:
	Restore ebx,esi,edi,ebp
	Exit

pj_svga_init endp

;*****************************************************************************
;* void pj_svga_cleanup(void)
;*   called just before driver is unloaded, does any needed cleanup.
;*****************************************************************************

pj_svga_cleanup proc near

	Entry
	Save	ebx,esi,edi,ebp
	mov	eax,vcleanup
	test	eax,eax
	jz	short #done
	call	eax
#done:
	Restore ebx,esi,edi,ebp
	Exit

pj_svga_cleanup endp

;*****************************************************************************
;* Errcode pj_svga_setmode(SMInfo *psmode)
;*   Sets the svga video mode described by *psmode.  Returns a negative
;*   error code on failure, or a longword value that will be passed to the
;*   corresponding clrmode() routine (if any) when the device is closed.
;*****************************************************************************

pj_svga_setmode proc near

	Entry
	Args	#psmode
	Save	ebx,esi,edi,ebp

	mov	eax,vsetmode		; get the setmode vector, if not NULL,
	test	eax,eax 		; go call the routine it points
	jnz	short #call_modeset	; to, else we have a "can't happen"
	mov	eax,err_no_display	; glitch, load the error status
	jmp	short #done		; and punt.

#call_modeset:
	mov	esi,#psmode		; load pointer to SMInfo for mode.
	lea	edi,pj_vdrv_wcontrol	; vectored setmode routines use this.
	movzx	ecx,[esi].smi_width	; most all cards will want the width,
	mov	[edi].wwidth,ecx	; pitch, and height values in the
	mov	[edi].wpitch,ecx	; wcontrol structure set the same as
	movzx	edx,[esi].smi_height	; the values in the SMInfo, so we do
	mov	[edi].wheight,edx	; that for them (they can override).
	call	eax			; call the set-mode routine.
#done:
	Restore ebx,esi,edi,ebp
	Exit

pj_svga_setmode endp

;*****************************************************************************
;* void pj_svga_clrmode(long old_mode_data) - restore prior video mode
;*****************************************************************************

pj_svga_clrmode proc near

	Entry
	Args	#old_mode_data
	Save	ebx,esi,edi,ebp

	mov	eax,vclrmode
	test	eax,eax
	jz	short #done

	mov	esi,#old_mode_data
	lea	edi,pj_vdrv_wcontrol
	call	eax
#done:
	Restore ebx,esi,edi,ebp
	Exit

pj_svga_clrmode endp

;*****************************************************************************
;* char *pj_svga_getname(void) - get the name of the video device we found.
;*****************************************************************************

pj_svga_getname proc near

	Entry

	mov	eax,vdevname
	test	eax,eax
	jnz	short #done
	lea	eax,vname_null
#done:
	Exit

pj_svga_getname endp

_TEXT	ends
	end

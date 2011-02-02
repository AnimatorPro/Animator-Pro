
	name	RECT
	subttl	rect.asm



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


	include compaq.i

;..............................................................................

_data	segment

	assume	ds:DGROUP

	extrn	pitch		    : dword
	extrn	write_vidbuf	    : dword
	extrn	bytes_in_window     : dword
	extrn	num_windows	    : dword
	extrn	bytes_left_over     : dword
	extrn	banks_per_window    : dword
	extrn	isolate_window_offset	    : dword
	extrn	anti_isolate_window_offset  : dword
	extrn	inc_isolate_window_offset   : dword
	extrn	isolate_bank_shifts	    : dword

wid	dd	320
hgt	dd	200

_data	ends

;..............................................................................

_text	segment

	assume	cs:CGROUP, ds:DGROUP


	extrn	_set_write_bank     : near


;******************************************************************************
;
;   void    set_rast(VRaster *r, Pixel color);
;
;	    fill the screen with a color
;
;******************************************************************************

set_rast    proc    near

#raster equ args
#color	equ args+4

	public	set_rast


	push	ebp
	mov	ebp, esp
	push	ebx
	push	edi
	push	es

	mov	ax, PHAR_REAL_SEG	; ES == prot mode segment of 1st Meg
	mov	es, ax			;	of memory

	mov	eax, #color		; EAX == flood color
	mov	ah, al
	movzx	ebx, ax
	shl	eax, 16
	or	eax, ebx

	mov	ebx, num_windows	; number of banks to flood completely
	mov	edx, 0			; first window

	cmp	ebx, 0			; if no flooded windows, then left over
	jz	#left_over


	; fill completely-filled windows

#loop:
	call	_set_write_bank 	; set the proper write bank

	mov	edi, write_vidbuf	; flood a window with EAX
	mov	ecx, bytes_in_window
	shr	ecx, 2
	rep	stosd

	add	edx, banks_per_window	; next window's bank
	dec	ebx			; window countdown
	jnz	#loop


	; fill incompletely-filled window

#left_over:

	mov	ecx, bytes_left_over	; if some bytes for the last window,
	jcxz	#exit

	call	_set_write_bank
	mov	edi, write_vidbuf	; then flood to end of screen
	rep	stosb

#exit:

	pop	es
	pop	edi
	pop	ebx
	pop	ebp
	ret

set_rast    endp

;******************************************************************************
;
;   void set_rect(VRaster *r,  Pixel color, Coor x, Coor y, Ucoor w, Ucoor h);
;   void set_hline(VRaster *r, Pixel color, Coor x, Coor y, Ucoor w);
;   void set_vline(VRaster *r, Pixel color, Coor x, Coor y, Ucoor h);
;
;******************************************************************************

set_rect    proc    near

	public	set_rect, set_hline, set_vline


#raster equ args
#color	equ args+4
#x	equ args+8
#y	equ args+12
#w	equ args+16
#h	equ args+20


	push	ebp
	mov	ebp,esp
	push	ebx
	push	edi
	push	esi
	push	es

	mov	eax, #h
	mov	hgt, eax
	mov	eax, #w
	mov	wid, eax

	jmp	#enter

set_hline:

	push	ebp
	mov	ebp,esp
	push	ebx
	push	edi
	push	esi
	push	es

	mov	hgt, 1
	mov	eax, #w
	mov	wid, eax

	jmp	#enter

set_vline:

	push	ebp
	mov	ebp,esp
	push	ebx
	push	edi
	push	esi
	push	es

	mov	eax, #w
	mov	hgt, eax
	mov	wid, 1

	jmp	#enter

	push	ebp
	mov	ebp,esp
	push	ebx
	push	edi
	push	esi
	push	es

#enter:

	cmp	wid, 0	      ; return if rect has no area
	je	#exit
	cmp	hgt, 0
	je	#exit

	mov	esi, #color

	mov	ax, PHAR_REAL_SEG
	mov	es, ax
	mov	ebx, pitch
	imul	ebx, #y
	add	ebx, #x 	; EBX = pixel offset in video buffer
	mov	edi, ebx	; EDI = pixel offset in video buffer
				; ES:EDI = pixel offset from bottom of memory

	mov	edx, edi	; start with correct video bank
	push	ecx
	mov	ecx, isolate_bank_shifts
	shr	edx, cl
	pop	ecx

	call	_set_write_bank

#start_row:
	mov	ecx, wid	; ECX = row length

	push	edi
#check_width:
	jecxz	#next_row	; check if row is done

	mov	edx, edi	; if not in the correct bank,
	push	ecx
	mov	ecx, isolate_bank_shifts
	shr	edx, cl
	pop	ecx

	call	_set_write_bank

	mov	edx, edi	; if this move crosses into another bank
	and	edx, isolate_window_offset
	add	edx, ecx
	dec	edx
	test	edx, anti_isolate_window_offset
	jz	#no_bank_cross

	push	ecx

	mov	ebx, edi	; compute bytes to end of bank
	and	ebx, isolate_window_offset
	mov	ecx, inc_isolate_window_offset
	sub	ecx, ebx
	mov	ebx, ecx	; and put it in ECX and EBX

	call	set_video_with_color

	add	edi, ebx

	pop	ecx

	sub	ecx, ebx	; decrement width by number of bytes just moved

	jmp	#check_width	; go move the remaining width

#no_bank_cross:

	call	set_video_with_color

#next_row:

	pop	edi
	add	edi, pitch

	dec	hgt
	jnz	#start_row

#exit:
	pop	es
	pop	esi
	pop	edi
	pop	ebx
	pop	ebp
	ret


	ret

set_rect    endp

;*****************************************************************************
;
;   flood ECX bytes at in write video (ES:EDI) with lowest byte in ESI
;
;*****************************************************************************

set_video_with_color  proc    near

	push	eax
	push	ecx
	push	edi

	mov	eax, esi		; get color
	mov	ah, al			; duplicate it for word moves

	and	edi, isolate_window_offset
	add	edi, write_vidbuf

	cld

	cmp	ecx, 8
	ja	svwc_long
	rep	stosb
	jmp	svwc_exit

svwc_long:
	test	edi, 1
	jz	svwc_now_even
	stosb
	dec	ecx

svwc_now_even:

	jecxz	svwc_exit

	push	ecx
	shr	ecx, 1
	rep	stosw
	pop	ecx

	test	ecx, 1
	jz	svwc_exit

	stosb

svwc_exit:
	pop	edi
	pop	ecx
	pop	eax

	ret

set_video_with_color  endp

;******************************************************************************

_text	ends

	end


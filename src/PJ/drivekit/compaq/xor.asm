	name	XOR
	subttl	xor.asm



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
	extrn	isolate_window_offset	    : dword
	extrn	anti_isolate_window_offset  : dword
	extrn	inc_isolate_window_offset   : dword
	extrn	isolate_bank_shifts	    : dword

_data	ends

;..............................................................................

_text	segment

	assume	cs:CGROUP, ds:DGROUP


	extrn	_set_write_bank     : near

;******************************************************************************
;
;   void xor_rect(VRaster *r, Pixel color, Coor x, Coor y, Ucoor w, Ucoor h);
;
;******************************************************************************

xor_rect    proc    near

	public	xor_rect


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

	cmp	dword ptr #w, 0 ; return if rect has no area
	je	#exit
	cmp	dword ptr #h, 0
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
	mov	ecx, #w 	; ECX = row length

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

	call	xor_video_with_color

	add	edi, ebx

	pop	ecx

	sub	ecx, ebx	; decrement width by number of bytes just moved

	jmp	#check_width	; go move the remaining width

#no_bank_cross:

	call	xor_video_with_color

#next_row:

	pop	edi
	add	edi, pitch

	dec	dword ptr #h
	jnz	#start_row

#exit:
	pop	es
	pop	esi
	pop	edi
	pop	ebx
	pop	ebp
	ret


	ret

xor_rect    endp

;*****************************************************************************
;
;   xor ECX bytes of read '(ES:ESI)' and write video (ES:EDI) with lowest byte
;   of ESI
;
;*****************************************************************************

xor_video_with_color proc   near

	push	eax
	push	ebx
	push	ecx
	push	edi
	push	es

	mov	ebx, esi	    ; get the color
	mov	bh, bl		    ; duplicate it for word moves

	push	esi

	mov	esi, edi
	and	esi, isolate_window_offset
	add	esi, write_vidbuf

	and	edi, isolate_window_offset
	add	edi, write_vidbuf

	cld

	cmp	ecx, 8
	ja	xvwc_long
xvwc_short_loop:
	mov	al, es:[esi]
	inc	esi
	xor	al, bl
	stosb
	loop	xvwc_short_loop
	jmp	xvwc_exit

xvwc_long:
	test	edi, 1
	jz	xvwc_now_even
	mov	al, es:[esi]
	inc	esi
	xor	al, bl
	stosb
	dec	ecx

xvwc_now_even:

	jecxz	xvwc_exit

	push	ecx
	shr	ecx, 1
xvwc_long_loop:
	mov	ax, es:[esi]
	add	esi, 2
	xor	ax, bx
	stosw
	loop	xvwc_long_loop
	pop	ecx

	test	ecx, 1
	jz	xvwc_exit

	mov	al, es:[esi]
	xor	al, bl
	stosb

xvwc_exit:

	pop	esi

	pop	es
	pop	edi
	pop	ecx
	pop	ebx
	pop	eax

	ret

xor_video_with_color endp

;******************************************************************************

_text	ends

	end

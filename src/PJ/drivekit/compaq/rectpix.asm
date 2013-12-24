
	name	RECTPIX
	subttl	rectpix.asm



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

wid	dd	320
hgt	dd	200
move	dd	offset mov_ram_to_video

_data	ends

;..............................................................................

_text	segment

	assume	cs:CGROUP, ds:DGROUP


	extrn	_set_write_bank     : near


;******************************************************************************
;
;   void put_rectpix(VRaster *r, void buf, Coor x, Coor y, Ucoor w, Ucoor h);
;   void put_hseg(VRaster *r, void buf, Coor x, Coor y, Ucoor w);
;   void put_vseg(VRaster *r, void buf, Coor x, Coor y, Ucoor h);
;   void get_rectpix(VRaster *r, void buf, Coor x, Coor y, Ucoor w, Ucoor h);
;   void get_hseg(VRaster *r, void buf, Coor x, Coor y, Ucoor w);
;   void get_vseg(VRaster *r, void buf, Coor x, Coor y, Ucoor h);
;
;******************************************************************************

put_rectpix proc    near

	public	put_rectpix, put_hseg, put_vseg
	public	get_rectpix, get_hseg, get_vseg


#raster equ args
#buf	equ args+4
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

	mov	eax, #w
	mov	wid, eax
	mov	eax, #h
	mov	hgt, eax
	mov	move, offset mov_ram_to_video

	jmp	#enter

put_vseg:

	push	ebp
	mov	ebp,esp
	push	ebx
	push	edi
	push	esi
	push	es

	mov	wid, 1
	mov	eax, #w
	mov	hgt, eax
	mov	move, offset mov_ramdot_to_video

	jmp	#enter

put_hseg:

	push	ebp
	mov	ebp,esp
	push	ebx
	push	edi
	push	esi
	push	es

	mov	eax, #w
	mov	wid, eax
	mov	hgt, 1
	mov	move, offset mov_ram_to_video

	jmp	#enter

get_rectpix:

	push	ebp
	mov	ebp,esp
	push	ebx
	push	edi
	push	esi
	push	es

	mov	eax, #w
	mov	wid, eax
	mov	eax, #h
	mov	hgt, eax
	mov	move, offset mov_video_to_ram

	jmp	#enter

get_vseg:

	push	ebp
	mov	ebp,esp
	push	ebx
	push	edi
	push	esi
	push	es

	mov	wid, 1
	mov	eax, #w
	mov	hgt, eax
	mov	move, offset mov_videodot_to_ram

	jmp	#enter

get_hseg:

	push	ebp
	mov	ebp,esp
	push	ebx
	push	edi
	push	esi
	push	es

	mov	eax, #w
	mov	wid, eax
	mov	hgt, 1
	mov	move, offset mov_video_to_ram

#enter:

	cmp	dword ptr wid, 0 ; return if rect has no area
	je	#exit
	cmp	dword ptr hgt, 0
	je	#exit

	mov	esi, #buf

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

	call	move

	add	edi, ebx

	pop	ecx

	sub	ecx, ebx	; decrement width by number of bytes just moved

	jmp	#check_width	; go move the remaining width

#no_bank_cross:

	call	move

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

put_rectpix endp

;*****************************************************************************
;
;   move ECX bytes of RAM (DS:ESI) to write video (ES:EDI)
;
;*****************************************************************************

mov_ram_to_video proc near

	push	ecx
	push	edi

	and	edi, isolate_window_offset
	add	edi, write_vidbuf

	cld

	cmp	ecx, 8
	ja	mrtv_long
	rep	movsb
	jmp	mrtv_exit

mrtv_long:
	test	edi, 1
	jz	mrtv_now_even
	movsb
	dec	ecx

mrtv_now_even:

	jecxz	mrtv_exit

	push	ecx
	shr	ecx, 1
	rep	movsw
	pop	ecx

	test	ecx, 1
	jz	mrtv_exit

	movsb

mrtv_exit:
	pop	edi
	pop	ecx

	ret

mov_ram_to_video endp

;*****************************************************************************
;
;   move ECX bytes of read video '(ES:EDI)' to RAM '(DS:ESI)'
;
;*****************************************************************************

mov_video_to_ram  proc	  near

	push	ecx
	push	edi
	push	ds
	push	es

	and	edi, isolate_window_offset
	add	edi, write_vidbuf

	cld

	push	es
	push	ds
	pop	es
	pop	ds

	xchg	esi, edi

	cmp	ecx, 8
	ja	mvtr_long
	rep	movsb
	jmp	mvtr_exit

mvtr_long:
	test	edi, 1
	jz	mvtr_now_even
	movsb
	dec	ecx

mvtr_now_even:

	jecxz	mvtr_exit

	push	ecx
	shr	ecx, 1
	rep	movsw
	pop	ecx

	test	ecx, 1
	jz	mvtr_exit

	movsb

mvtr_exit:

	xchg	esi, edi

	pop	es
	pop	ds
	pop	edi
	pop	ecx

	ret

mov_video_to_ram  endp

;******************************************************************************

mov_ramdot_to_video proc    near

	push	edi

	and	edi, isolate_window_offset
	add	edi, write_vidbuf

	cld
	rep	movsb

	pop	edi

	ret

mov_ramdot_to_video endp

;******************************************************************************

mov_videodot_to_ram proc    near


	push	edi
	push	ds
	push	es

	and	edi, isolate_window_offset
	add	edi, write_vidbuf

	cld

	push	es
	push	ds
	pop	es
	pop	ds

	xchg	esi, edi

	movsb

	xchg	esi, edi

	pop	es
	pop	ds
	pop	edi

	ret

mov_videodot_to_ram endp

;******************************************************************************

_text	ends

	end

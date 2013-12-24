
	name	DOT
	subttl	dot.asm



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

	extrn	pitch			: dword
	extrn	write_bank		: word
	extrn	write_vidbuf		: dword
	extrn	isolate_bank_shifts	: dword
	extrn	isolate_window_offset	: dword
	extrn	VESAModeBlock		: byte

_data	ends

;..............................................................................

_text	segment

	assume	cs:CGROUP, ds:DGROUP


	extrn	_set_write_bank     : near


;******************************************************************************
;
;   void    put_dot(VRaster *r, Pixel color, Coor x, Coor y);
;
;	    put a dot on the screen with a color
;
;******************************************************************************

cput_dot    proc    near

	public	cput_dot, put_dot


#raster equ args
#color	equ args+4
#x	equ args+8
#y	equ args+12

	mov	eax, [esp+12]
	cmp	eax, 0
	jl	#cpd_ret
	cmp	ax, VESAModeBlock.ms_xsize
	jge	#cpd_ret
	mov	eax, [esp+16]
	cmp	eax, 0
	jl	#cpd_ret
	cmp	ax, VESAModeBlock.ms_ysize
	jge	#cpd_ret

put_dot:

	push	ebp
	mov	ebp, esp
	push	ebx
	push	es

	mov	ebx, pitch
	imul	ebx, #y
	add	ebx, #x
	mov	edx, ebx
	mov	ecx, isolate_bank_shifts
	shr	edx, cl
	cmp	dx, write_bank
	je	#write_bank_ok
	call	_set_write_bank
#write_bank_ok:
	and	ebx, isolate_window_offset
	add	ebx, write_vidbuf
	mov	ax, PHAR_REAL_SEG
	mov	es, ax

	mov	al, #color
	mov	es:[ebx], al

	pop	es
	pop	ebx
	pop	ebp
#cpd_ret:
	ret

cput_dot    endp

;******************************************************************************
;
;   Pixel   get_dot(VRaster *r, Coor x, Coor y);
;
;	    put a dot on the screen with a color
;
;******************************************************************************

cget_dot    proc    near

	public	cget_dot, get_dot


#raster equ args
#x	equ args+4
#y	equ args+8

	mov	eax, [esp+8]
	cmp	eax, 0
	jl	#cgd_err
	cmp	ax, VESAModeBlock.ms_xsize
	jge	#cgd_err
	mov	eax, [esp+12]
	cmp	eax, 0
	jl	#cgd_err
	cmp	ax, VESAModeBlock.ms_ysize
	jge	#cgd_err

get_dot:

	push	ebp
	mov	ebp, esp
	push	ebx
	push	es

	mov	ebx, pitch
	imul	ebx, #y
	add	ebx, #x
	mov	edx, ebx
	mov	ecx, isolate_bank_shifts
	shr	edx, cl
	cmp	dx, write_bank
	je	#write_bank_ok
	call	_set_write_bank
#write_bank_ok:
	and	ebx, isolate_window_offset
	add	ebx, write_vidbuf
	mov	ax, PHAR_REAL_SEG
	mov	es, ax

	mov	al, es:[ebx]
	and	eax, 0ffh


	pop	es
	pop	ebx
	pop	ebp
	ret

#cgd_err:
	xor	eax, eax
	ret

cget_dot    endp

;******************************************************************************

_text	ends

	end

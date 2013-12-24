
	name	UNCOMP
	subttl	uncomp.asm



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

_data	ends

;..............................................................................

_text	segment

	assume	cs:CGROUP, ds:DGROUP


	extrn	_set_write_bank     : near


;******************************************************************************
;
;   void uncc64(VRaster *r, void *cbuf);
;
;******************************************************************************

uncc64	proc	near

	public	uncc64


#raster equ	args
#cbuf	equ	args+4


	push	ebp
	mov	ebp, esp
	push	ebx
	push	ecx
	push	edx
	push	edi
	push	esi

	xor	eax, eax
	xor	ebx, ebx
	xor	ecx, ecx

	mov	esi, #cbuf		; ESI == chunk to decode

	lodsw				; EDI == # of strips of color
	mov	edi, eax
	cmp	edi, 0
	jz	v64_exit

v64_next_strip:

	lodsb				; BH == current index of strip of colors
	add	bh, al
	mov	bl, bh

	lodsb				; ECX == number of colors in the strip
	mov	cl, al
	cmp	cl, 0			; if ECX == 0, then all 256 colors
	jnz	v64_load_palette
	mov	ecx, 256

v64_load_palette:

	mov	al, bl			; set the color index
	mov	dx, 3c8h
	out	dx, al
	inc	dx

	lodsb
	out	dx, al			; red
	lodsb
	out	dx, al			; green
	lodsb
	out	dx, al			; blue

	inc	bl			; next color

	loop	v64_load_palette

	dec	edi
	jnz	v64_next_strip

v64_exit:

	pop	esi
	pop	edi
	pop	edx
	pop	ecx
	pop	ebx
	pop	ebp

	ret

uncc64 endp

;******************************************************************************
;
;   void uncc256(VRaster *r, void *cbuf);
;
;******************************************************************************

uncc256 proc   near

	public	uncc256


#raster equ	args
#cbuf	equ	args+4


	push	ebp
	mov	ebp, esp
	push	ebx
	push	ecx
	push	edx
	push	edi
	push	esi

	xor	eax, eax
	xor	ebx, ebx
	xor	ecx, ecx

	mov	esi, #cbuf		; ESI == chunk to decode

	lodsw				; EDI == # of strips of color
	mov	edi, eax
	cmp	edi, 0
	jz	v256_exit

v256_next_strip:

	lodsb				; BH == current index of strip of colors
	add	bh, al
	mov	bl, bh

	lodsb				; ECX == number of colors in the strip
	mov	cl, al
	cmp	cl, 0			; if ECX == 0, then all 256 colors
	jnz	v256_load_palette
	mov	ecx, 256

v256_load_palette:

	mov	al, bl			; set the color index
	mov	dx, 3c8h
	out	dx, al
	inc	dx

	lodsb
	shr	al, 2
	out	dx, al			; red
	lodsb
	shr	al, 2
	out	dx, al			; green
	lodsb
	shr	al, 2
	out	dx, al			; blue

	inc	bl			; next color

	loop	v256_load_palette

	dec	edi
	jnz	v256_next_strip

v256_exit:

	pop	esi
	pop	edi
	pop	edx
	pop	ecx
	pop	ebx
	pop	ebp

	ret

uncc256 endp

;******************************************************************************

_text	ends

	end

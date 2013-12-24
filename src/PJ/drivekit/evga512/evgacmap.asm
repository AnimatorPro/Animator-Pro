; Set the color palette
           include evga.i

_text	segment	para public use32 'code'
	assume cs:CGROUP

;void evga_set_colors(void *v, int startix, int count, Color *cmap);
;parameters:
;	void *v - ignored here.  Pointer to "raster" structure.
;	int startix - start color index to set.  (0-255)
;	int count - # of colors to set.
;	Color *cmap - 3 bytes for each color to set.  RGB values in
;			range 0-255
	public evga_set_colors
evga_set_colors proc near
jscp	struc	;evga_set_colors parameter structure
	jsc_ebp dd ?
	jsc_ret dd ?
	jsc_v	dd ?
	jsc_startix dd ?
	jsc_count dd ?
	jsc_cmap dd ?
jscp	ends
	push ebp
	mov ebp,esp
	push ebx
	push esi

	mov ebx,[ebp].jsc_startix
	mov ecx,[ebp].jsc_count
	mov esi,[ebp].jsc_cmap

	mov	dx,3c8h
	mov al,bl
	out dx,al
	inc dx
	jmp st1
st1:
	lodsb
	shr al,2	;convert red from 0-255 to 0-63
	out dx,al
	;jmp l1		;slow down the proc as per IBM spec.  Doesn't seem to
			;help with our bug one way or another.
;l1:
	lodsb
	shr al,2	;convert green from 0-255 to 0-63
	out dx,al
	;jmp l2
;l2:
	lodsb
	shr al,2	;convert blue from 0-255 to 0-63
	out dx,al
	;jmp l3
;l3:
	loop st1

	pop esi
	pop ebx
	pop ebp
	ret
evga_set_colors endp


_text	ends
	end

	include vram.i

_text	segment	para public use32 'code'
	assume cs:CGROUP

;void vram_set_colors(void *v, int startix, int count, Color *cmap);
	public vram_set_colors
vram_set_colors proc near
jscp	struc	;vram_set_colors parameter structure
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

;st1:
	mov	dx,3c8h
	mov al,bl
	out dx,al
	inc dx
st1:
	lodsb
	shr al,2
	out dx,al
	jmp st2
st2:
	lodsb
	shr al,2
	out dx,al
	jmp st3
st3:
	lodsb
	shr al,2
	out dx,al
	loop st1

	pop esi
	pop ebx
	pop ebp
	ret
vram_set_colors endp

;void ovram_set_colors(void *v, int startix, int count, Color *cmap);
	public ovram_set_colors
scop struc
	sco_ebp dd ?
	sco_ret dd ?
	sco_v dd ?
	sco_start dd ?
	sco_count dd ?
	sco_cmap dd ?
scop ends
ovram_set_colors proc near
	push ebp
	mov ebp,esp
	push esi

	mov	edx,3c8h
	mov eax,[ebp].sco_start
	out dx,al
	inc edx

	mov esi,[ebp].sco_cmap
	mov eax,[ebp].sco_count
	mov ecx,eax
	add ecx,eax
	add ecx,eax
	rep outsb

	pop esi
	pop ebp
	ret
ovram_set_colors endp


_text	ends
	end

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

;void pj_mcga_set_colors(void *v, int startix, int count, Color *cmap);
	public pj_mcga_set_colors
pj_mcga_set_colors proc near
jscp	struc	;pj_mcga_set_colors parameter structure
	jsc_esi dd ?
	jsc_edx dd ?
	jsc_ecx dd ?
	jsc_ebx dd ?
	jsc_ret	dd ?
	jsc_v	dd ?
	jsc_startix dd ?
	jsc_count dd ?
	jsc_cmap dd ?
jscp	ends

	push ebx
	push ecx
	push edx
	push esi

	mov ebx,[esp].jsc_startix
	mov ecx,[esp].jsc_count
	mov esi,[esp].jsc_cmap

st1:
	mov	dx,3c8h
	mov al,bl
	out dx,al
	inc bl
	inc dx
	jmp s1
s1:
	lodsb
	shr al,2
	out dx,al
	jmp s2
s2:
	lodsb
	shr al,2
	out dx,al
	jmp s3
s3:
	lodsb
	shr al,2
	out dx,al
	loop st1

	pop esi
	pop edx
	pop ecx
	pop ebx
	ret

pj_mcga_set_colors endp

code	ends
	end

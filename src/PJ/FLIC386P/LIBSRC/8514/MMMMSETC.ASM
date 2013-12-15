

	include a8514.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

;void pj_8514_set_colors(void *v, int startix, int count, Color *cmap);
	public pj_8514_set_colors
scop struc
	sco_ebp dd ?
	sco_ret dd ?
	sco_v dd ?
	sco_start dd ?
	sco_count dd ?
	sco_cmap dd ?
scop ends
pj_8514_set_colors proc near
	push ebp
	mov ebp,esp
	push esi

	mov	edx,DAC_WINDEX
	mov eax,[ebp].sco_start
	out dx,al
	inc edx

	mov esi,[ebp].sco_cmap
	mov ecx,[ebp].sco_count

st1:
	lodsb
	shr al,2
	out dx,al
	lodsb
	shr al,2
	out dx,al
	lodsb
	shr al,2
	out dx,al
	loop st1

	pop esi
	pop ebp
	ret
pj_8514_set_colors endp

code	ends
	end

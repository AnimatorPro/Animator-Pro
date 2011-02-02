
	include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


;void JREGS pj_shift_cmap(UBYTE *s, UBYTE *d, int count);
	public pj_shift_cmap
pj_shift_cmap proc near
shcp struc
	shc_ebp dd ?
	shc_ret dd ?
	shc_s dd ?
	shc_d dd ?
	shc_count dd ?
shcp ends
	push ebp
	mov ebp,esp
	push ecx
	push esi
	push edi

	mov esi,[ebp].shc_s
	mov edi,[ebp].shc_d
	mov ecx,[ebp].shc_count

shc_loop:
	lodsb
	shl al,2
	stosb
	loop shc_loop

	pop edi
	pop esi
	pop ecx
	pop ebp
	ret
pj_shift_cmap endp

code	ends
	end

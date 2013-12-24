
	include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


	public blit
;void JREGS blit(Vscreen *sv, int sx, int sy, Vscreen *dv, int dx, int dy,
;	int w, int h);

;Blit a bitplane into memory byte-a-pixel plane.
blit proc near
bli8p	struc	;pj_bli8 parameter structure
	bli8_edi dd ?	;whats there from pushad
	bli8_esi dd ?
	bli8_ebp dd ?
	bli8_esp dd ?
	bli8_ebx dd ?
	bli8_edx dd ?
	bli8_ecx dd ?
	bli8_eax dd ?
	bli8_ret dd ?	;return address for function
	bli8_sv	dd ?
	bli8_sx	dd ?
	bli8_sy	dd ?
	bli8_dv	dd ?
	bli8_dx	dd ?
	bli8_dy	dd ?
	bli8_w	dd ?
	bli8_h	dd ?
bli8p ends
	pushad
	mov ebp,esp
	push es
	push ds
lvarspace equ 4
pushspace equ 8
dmod equ	dword ptr [ebp-pushspace-4]
	sub esp,lvarspace	;space for local variables

	;load destination screen pointer and clip
	mov edi,[ebp].bli8_dv

	;put destination start address into es:edi 
	mov ebx,[edi].bym_bpr
	mov eax,[ebp].bli8_dy
	mul ebx
	sub ebx,[ebp].bli8_w
	mov dmod,ebx		;dest modulo in dmod
	mov bx,[edi].bym_pseg
	add eax,[ebp].bli8_dx
	mov edi,[edi].bym_p
	add edi,eax
	mov es,bx

here:
	;put source start address into ds:esi 
	mov esi,[ebp].bli8_sv
	mov ebx,[esi].bym_bpr
	mov eax,[ebp].bli8_sy
	mul ebx
	sub ebx,[ebp].bli8_w
	mov ecx,ebx		;source modula in eax
	mov bx,[esi].bym_pseg
	add eax,[ebp].bli8_sx
	mov esi,[esi].bym_p
	add esi,eax
	mov ds,bx

	mov eax,ecx
	mov ebx,[ebp].bli8_h
	mov edx,[ebp].bli8_w
bli8_line:
	mov ecx,edx
	public here
	rep movsb
	add edi,dmod
	add esi,eax
	dec	ebx
	jnz bli8_line



blit8_out:
	add esp,lvarspace	;clean off local variables
	pop ds
	pop es
	popad
	ret
blit endp


code	ends
	end

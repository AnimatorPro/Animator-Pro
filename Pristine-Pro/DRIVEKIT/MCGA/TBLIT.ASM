
	include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


	public tblit
;void JREGS tblit(Vscreen *sv, int sx, int sy, Vscreen *dv, int dx, int dy,
;	int w, int h, int tcolor);
;Blit a bitplane into memory byte-a-pixel plane.
tblit proc near
tbl8p	struc	;tbli8 parameter structure
	tbl8_edi dd ?	;what's there from pushad
	tbl8_esi dd ?
	tbl8_ebp dd ?
	tbl8_esp dd ?
	tbl8_ebx dd ?
	tbl8_edx dd ?
	tbl8_ecx dd ?
	tbl8_eax dd ?
	tbl8_ret dd ?	;return address for function
	tbl8_sv	dd ?
	tbl8_sx	dd ?
	tbl8_sy	dd ?
	tbl8_dv	dd ?
	tbl8_dx	dd ?
	tbl8_dy	dd ?
	tbl8_w	dd ?
	tbl8_h	dd ?
	tbl8_tcolor	dd ?
tbl8p ends
	pushad
	mov ebp,esp
	push es
	push ds
lvarspace equ 16
pushspace equ 8
smod equ	dword ptr [ebp-pushspace-4]
dmod equ dword ptr [ebp-pushspace-8]
	sub esp,lvarspace	;space for local variables

	;load destination screen pointer and clip
	mov edi,[ebp].tbl8_dv

	;put destination start address into es:edi 
	mov ebx,[edi].bym_bpr
	mov eax,[ebp].tbl8_dy
	mul ebx
	sub ebx,[ebp].tbl8_w
	mov dmod,ebx
	mov bx,[edi].bym_pseg
	add eax,[ebp].tbl8_dx
	mov edi,[edi].bym_p
	add edi,eax
	mov es,bx

	;put source start address into ds:esi 
	mov esi,[ebp].tbl8_sv
	mov ebx,[esi].bym_bpr
	mov eax,[ebp].tbl8_sy
	mul ebx
	sub ebx,[ebp].tbl8_w
	mov smod,ebx
	mov bx,[esi].bym_pseg
	add eax,[ebp].tbl8_sx
	mov esi,[esi].bym_p
	add esi,eax
	mov ds,bx

	mov ebx,[ebp].tbl8_h
	mov edx,[ebp].tbl8_w
	mov eax,[ebp].tbl8_tcolor
	mov ah,al

tbl8_line:
	mov ecx,edx
tblilp:
	lodsb
	cmp al,ah
	jz  skip
	stosb
	loop tblilp
	jmp endline
skip:
	inc edi
	loop tblilp
endline:
	add edi,dmod
	add esi,smod
	dec	ebx
	jnz tbl8_line

tblit8_out:
	add esp,lvarspace	;clean off local variables
	pop ds
	pop es
	popad
	ret
tblit endp


code	ends
	end

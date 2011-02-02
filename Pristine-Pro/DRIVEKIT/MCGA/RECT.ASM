	include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


cblp struc	;mcga_set_rect/mcga_xor_rect parameter structure
	cbl_edi dd ?
	cbl_esi dd ?
	cbl_ebp dd ?
	cbl_esp dd ?
	cbl_ebx dd ?
	cbl_edx dd ?
	cbl_ecx dd ?
	cbl_eax dd ?
	cbl_ret dd ?
	cbl_v	dd ?
	cbl_color dd ?
	cbl_x	dd ?
	cbl_y	dd ?
	cbl_w	dd ?
	cbl_h	dd ?
cblp ends


	public mcga_set_rect
;void JREGS mcga_set_rect(Bytemap *bm,  RCOLOR color, 
;			 LONG x, LONG y, ULONG width, ULONG height);
mcga_set_rect proc near
	pushad
	mov ebp,esp
	push es

	mov esi,[ebp].cbl_v

	;address of upper left pixel into es:di
	mov eax,[ebp].cbl_y
	mov ebx,[esi].bym_bpr
	mul ebx
	mov edi,[ebp].cbl_x
	add edi,eax
	add edi,[esi].bym_p
	mov ax,VGA_SEG
	mov es,ax

	sub ebx,[ebp].cbl_w	;ebx is what to add to get to next line
	mov esi,[ebp].cbl_h	
	mov eax,[ebp].cbl_color
	jmp zcbz

cbloop:
	mov ecx,[ebp].cbl_w
	rep stosb
	add edi,ebx
zcbz:
	dec esi
	js cblockout
	jmp cbloop
cblockout:
	pop es
	popad
	ret
mcga_set_rect endp

	public mcga_xor_rect
;static void mcga_xor_rect(Bytemap *bm, RCOLOR color,
;				 LONG x, LONG y, ULONG width, ULONG height);
mcga_xor_rect proc near
	pushad
	mov ebp,esp
	push es
	push ds

	mov esi,[ebp].cbl_v

	;address of upper left pixel into es:di
	mov eax,[ebp].cbl_y
	mov ebx,[esi].bym_bpr
	mul ebx
	mov edi,[ebp].cbl_x
	add edi,eax
	add edi,[esi].bym_p
	mov ax,VGA_SEG
	mov es,ax
	mov ds,ax

	sub ebx,[ebp].cbl_w	;ebx is what to add to get to next line
	mov eax,[ebp].cbl_color
	mov ah,al
	mov esi,edi
	jmp xorzcbz

xorbloop:
	mov ecx,[ebp].cbl_w
	jecxz xorzi
xori:
	lodsb
	xor al,ah
	stosb
	loop xori
xorzi:
	add edi,ebx
	add esi,ebx
xorzcbz:
	dec [ebp].cbl_h
	js xorcblockout
	jmp xorbloop

xorcblockout:
	pop ds
	pop es
	popad
	ret
mcga_xor_rect endp


code	ends
	end

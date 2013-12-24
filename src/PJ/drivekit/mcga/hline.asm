
	include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

	public mcga_d_hline
;void mcga_d_hline(Bytemap *bm, RCOLOR color, LONG minx, 
;	LONG y, LONG width);
mcga_d_hline proc near
chlp struc  ;mcga_d_hline parameter structure
	chl_ebp	dd ?
	chl_ret dd ?
	chl_v	dd ?
	chl_color dd ?
	chl_x	dd ?
	chl_y	dd ?
	chl_w	dd ?
chlp ends
	push ebp
	mov ebp,esp
	push edx
	push ecx
	push edi
	push es

	;move start address of line into es:edi
	mov edi,[ebp].chl_v
	mov ax,VGA_SEG
	mov es,ax
	mov eax,[ebp].chl_y
	mul [edi].bym_bpr
	mov edi,[edi].bym_p
	add edi,eax
	add edi,[ebp].chl_x

	mov ecx,[ebp].chl_w
	mov eax,[ebp].chl_color
	rep stosb

	pop es
	pop edi
	pop ecx
	pop edx
	pop ebp
	ret
mcga_d_hline endp

code	ends
	end

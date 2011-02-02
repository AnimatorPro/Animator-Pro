
	include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

;LONG mcga_get_vseg(Bytemap *r,void *pixbuf, ULONG x,ULONG y,ULONG height);
	public mcga_get_vseg
mcga_get_vseg proc near
bgvp	struc	;stack frame
	bgv_ebp dd ?
	bgv_ret	dd ?
	bgv_r	dd ?
	bgv_pixbuf dd ?
	bgv_x	dd ?
	bgv_y	dd ?
	bgv_height dd ?
bgvp ends
	push ebp
	mov ebp,esp
	push ebx
	push ecx
	push edx
	push esi
	push edi
	push ds

	mov esi,[ebp].bgv_r		;fetch screen pointer
	mov edi,[ebp].bgv_x
	mov eax,[ebp].bgv_y

;calculate address of 1st pixel into ds:esi and leave bpr in ebx
	mov ebx,[esi].bym_bpr
	mul ebx
	add edi,eax
	add edi,[esi].bym_p
	mov ax,VGA_SEG
	mov ds,ax
	mov esi,edi

	mov edi,[ebp].bgv_pixbuf		;fetch destination
	dec ebx							;movsb will take care of 1 of bpr
	mov ecx,[ebp].bgv_height

bgv_001:
	movsb
	add esi,ebx
	loop bgv_001

bgv_out:
	pop ds
	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx
	pop ebp
	ret
mcga_get_vseg endp


code	ends
	end

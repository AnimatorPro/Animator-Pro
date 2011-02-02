
	include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


;void mcga_get_hseg(Bytemap *r,void *pixbuf, ULONG x,ULONG y,ULONG width);
	public mcga_get_hseg
mcga_get_hseg proc near
_mghp	struc	;stack frame
	_mgh_ebp dd ?
	_mgh_ret	dd ?
	_mgh_r	dd ?
	_mgh_pixbuf dd ?
	_mgh_x	dd ?
	_mgh_y	dd ?
	_mgh_width dd ?
_mghp ends
	push ebp
	mov ebp,esp
	push ecx
	push edx
	push esi
	push edi
	push ds

	;calculate pixel offset into eax
	mov esi,[ebp]._mgh_r	
	mov eax,[ebp]._mgh_y
	mul [esi].bym_bpr
	add eax,[ebp]._mgh_x
	;point es:esi to screen start
	mov esi,[esi].bym_p
	add esi,eax
	mov ax,VGA_SEG
	mov ds,ax
	mov edi,[ebp]._mgh_pixbuf
	mov ecx,[ebp]._mgh_width
	mov eax,ecx 					;save width as return value in eax

	rep movsb

	pop ds
	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebp
	ret
mcga_get_hseg endp

;void mcga_put_hseg(Bytemap *r,void *pixbuf,
;	ULONG x,ULONG y,ULONG width);
	public mcga_put_hseg
mcga_put_hseg proc near
_mphp	struc	;stack frame
	_mph_ebp dd ?
	_mph_ret	dd ?
	_mph_r	dd ?
	_mph_pixbuf dd ?
	_mph_x	dd ?
	_mph_y	dd ?
	_mph_width dd ?
_mphp ends
	push ebp
	mov ebp,esp
	push ecx
	push edx
	push esi
	push edi
	push es

	mov esi,[ebp]._mph_r		;fetch screen pointer
;calculate address of 1st pixel into es:edi
	mov ax,VGA_SEG
	mov es,ax
	mov eax,[ebp]._mph_y
	mul [esi].bym_bpr
	add eax,[ebp]._mph_x
	add eax,[esi].bym_p
	mov edi,eax

	mov esi,[ebp]._mph_pixbuf		;fetch source
	mov ecx,[ebp]._mph_width
	mov eax,ecx 					;save width as return value in eax

	rep movsb

_mph_out:
	pop es
	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebp
	ret
mcga_put_hseg endp


code	ends
	end

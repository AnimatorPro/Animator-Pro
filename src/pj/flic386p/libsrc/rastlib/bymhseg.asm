
	include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

;LONG pj_bym_get_hseg(Bytemap *r,void *pixbuf, ULONG x,ULONG y,ULONG width);
;unclipped! get horizontal line segment
	public pj_bym_get_hseg
pj_bym_get_hseg proc near
_bghp	struc	;stack frame
	_bgh_ebp dd ?
	_bgh_ret	dd ?
	_bgh_r	dd ?
	_bgh_pixbuf dd ?
	_bgh_x	dd ?
	_bgh_y	dd ?
	_bgh_width dd ?
_bghp ends
	push ebp
	mov ebp,esp
	push ecx
	push edx
	push esi
	push edi

	;calculate pixel offset into eax
	mov esi,[ebp]._bgh_r	
	mov eax,[ebp]._bgh_y
	mul [esi].bym_bpr
	add eax,[ebp]._bgh_x
	;point es:esi to screen start
	mov esi,[esi].bym_p
	add esi,eax
	mov edi,[ebp]._bgh_pixbuf
	mov ecx,[ebp]._bgh_width
	mov eax,ecx 					;save width as return value in eax

	rep movsb

	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebp
	ret
pj_bym_get_hseg endp


;LONG JREGS pj_bym_put_hseg(Bytemap *r,void *pixbuf,
;	ULONG x,ULONG y,ULONG width);
	public pj_bym_put_hseg
pj_bym_put_hseg proc near
_bphp	struc	;stack frame
	_bph_ebp dd ?
	_bph_ret	dd ?
	_bph_r	dd ?
	_bph_pixbuf dd ?
	_bph_x	dd ?
	_bph_y	dd ?
	_bph_width dd ?
_bphp ends
	push ebp
	mov ebp,esp
	push ecx
	push edx
	push esi
	push edi
	push es

	mov esi,[ebp]._bph_r		;fetch screen pointer
;calculate address of 1st pixel into es:edi
	mov eax,[ebp]._bph_y
	mul [esi].bym_bpr
	add eax,[ebp]._bph_x
	add eax,[esi].bym_p
	mov edi,eax

	mov esi,[ebp]._bph_pixbuf		;fetch source
	mov ecx,[ebp]._bph_width
	mov eax,ecx 					;save width as return value in eax

	rep movsb

_bph_out:
	pop es
	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebp
	ret
pj_bym_put_hseg endp


code	ends
	end

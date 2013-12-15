
	include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

;LONG pj_bym_get_vseg(Bytemap *r,void *pixbuf, ULONG x,ULONG y,ULONG height);
	public pj_bym_get_vseg
pj_bym_get_vseg proc near
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

;some clipping.
	;check x to be off screen to right and leave x in edi
	mov edi,[ebp].bgv_x
	cmp di,[esi].bym_w
	jae bgv_clippedout
	;check y to be off screen to right and leave y in eax, screenheight in edx
	mov eax,[ebp].bgv_y
	xor edx,edx
	mov dx,[esi].bym_h
	cmp eax,edx
	jae bgv_clippedout

	;clip height of line to move and leave in ecx
	mov ecx,[ebp].bgv_height
	add ecx,eax
	cmp ecx,edx
	jbe bgv_000
	mov ecx,edx
bgv_000: 
	sub ecx,eax

;calculate address of 1st pixel into ds:esi and leave bpr in ebx
	mov ebx,[esi].bym_bpr
	mul ebx
	add edi,eax
	add edi,[esi].bym_p
	mov ax,[esi].bym_pseg
	mov ds,ax
	mov esi,edi

	mov edi,[ebp].bgv_pixbuf		;fetch destination
	dec ebx							;movsb will take care of 1 of bpr
	mov eax,ecx 					;save height as return value in eax

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
bgv_clippedout:
	xor eax,eax		;return 0
	jmp bgv_out	
pj_bym_get_vseg endp


;LONG JREGS pj_bym_put_vseg(Bytemap *r,void *pixbuf,
;	ULONG x,ULONG y,ULONG height);
	public pj_bym_put_vseg
pj_bym_put_vseg proc near
bpvp	struc	;stack frame
	bpv_ebp dd ?
	bpv_ret	dd ?
	bpv_r	dd ?
	bpv_pixbuf dd ?
	bpv_x	dd ?
	bpv_y	dd ?
	bpv_height dd ?
bpvp ends
	push ebp
	mov ebp,esp
	push ebx
	push ecx
	push edx
	push esi
	push edi
	push es

	mov esi,[ebp].bpv_r		;fetch screen pointer
	mov ax,[esi].bym_pseg	;make extra segment point to screen pixels
	mov es,ax

;some clipping.
	;check x to be off screen to right and leave x in esi
	mov edi,[ebp].bpv_x
	cmp di,[esi].bym_w
	jae bpv_clippedout
	;check y to be off screen to right and leave y in eax, screen height in edx
	mov eax,[ebp].bpv_y
	xor edx,edx
	mov dx,[esi].bym_h
	cmp eax,edx
	jae bpv_clippedout

	;clip height of line to move and leave in ecx
	mov ecx,[ebp].bpv_height
	add ecx,eax
	cmp ecx,edx
	jbe bpv_000
	mov ecx,edx
bpv_000: 
	sub ecx,eax

;calculate address of 1st pixel into es:edi.  Leave bpr in ebx
	mov ebx,[esi].bym_bpr
	mul ebx
	add edi,eax
	add edi,[esi].bym_p

	mov esi,[ebp].bpv_pixbuf		;fetch source
	mov eax,ecx 					;save width as return value in eax
	dec ebx							;movsb subtracts one from bpr

bpr_001:
	movsb
	add edi,ebx
	loop bpr_001

bpv_out:
	pop es
	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx
	pop ebp
	ret
bpv_clippedout:
	xor eax,eax		;return 0
	jmp bpv_out	
pj_bym_put_vseg endp

code	ends
	end

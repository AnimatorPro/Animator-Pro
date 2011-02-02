
	include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


	public pj_bli1
;extern void pj_bli1(Vscreen *dv, int w, int h, int sx, int sy, int dx, dy, 
;	UBYTE *sp, int sbpr, int color);
;Blit a bitplane into memory byte-a-pixel plane.
pj_bli1 proc near
bli1p	struc	;pj_bli1 parameter structure
	bli1_edi dd ?	;what's there from pushad
	bli1_esi dd ?
	bli1_ebp dd ?
	bli1_esp dd ?
	bli1_ebx dd ?
	bli1_edx dd ?
	bli1_ecx dd ?
	bli1_eax dd ?
	bli1_ret dd ?	;return address for function
	bli1_dv	dd ?	;1st parameter - destination screen
	bli1_dx	dd ?
	bli1_dy	dd ?
	bli1_w	dd ?
	bli1_h	dd ?
	bli1_sx	dd ?
	bli1_sy	dd ?
	bli1_sp	dd ?
	bli1_sbpr dd ?
	bli1_color dd ?
bli1p ends
	pushad
	mov ebp,esp
	push es
lvarspace equ 16
pushspace equ 4
spt	equ	dword ptr [ebp-pushspace-4]
dpt equ dword ptr [ebp-pushspace-8]
	sub esp,lvarspace	;space for local variables

	mov edi,[ebp].bli1_dv	;get dest screen structure

	;get starting source address in spt
	mov	eax,[ebp].bli1_sy
	mul	[ebp].bli1_sbpr	;y line offset in ax
	mov	ebx,[ebp].bli1_sx
	shr	ebx,3	; += (sx1>>3)
	add	eax,ebx	;start source offset in ax
	add eax,[ebp].bli1_sp
	mov spt,eax



	;get starting destination address in es:dpt
	mov eax,[ebp].bli1_dy
	xor ebx,ebx
	mov ebx,[edi].bym_bpr
	mul ebx

	add eax,[edi].bym_p
	add eax,[ebp].bli1_dx
	mov dpt,eax
	mov ax,[edi].bym_pseg
	mov es,ax


	;calculate start mask for line into dl
	mov ecx,[ebp].bli1_sx
	and ecx,7
	mov dl,80h
	shr	dl,cl	;and devote dl to it...

	mov	eax,[ebp].bli1_color
	jmp	zabline
abline:
	mov	ecx,[ebp].bli1_w	;dot count in ecx
	mov	dh,dl		;get mask into dh
	mov	esi,spt
	mov	edi,dpt
	mov	ah,[esi]		;fetch 1st byte of source into ah
	inc	esi
abpix:
	test	ah,dh
	jnz	abset
	inc	edi	;skip pixel in dest
	shr	dh,1
	jz	newsrc
	loop	abpix
	jmp	zline
abset:	stosb		;set pixel in dest
	shr	dh,1
	jz	newsrc
	loop	abpix
zline:	mov	ecx,[ebp].bli1_sbpr
	add	spt,ecx
	add	dpt,ebx
zabline:	dec	[ebp].bli1_h
	js	za1
	jmp	abline

newsrc:	;get next byte of source
	mov	ah,[esi]		;fetch byte of source into ah
	inc	esi
	mov	dh,80h		;mask to 1st pixel in byte
	loop	abpix
	jmp	zline

za1:

	add esp,lvarspace	;clear off local variables
	pop es
	popad
	ret

pj_bli1 endp


code	ends
	end


	include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP



	public pj_bli2
;extern void pj_bli2(Vscreen *dv, int w, int h, int sx, int sy, int dx, dy, 
;	UBYTE *sp, int sbpr, int color, int bcolor);
;Blit a bitplane into memory byte-a-pixel plane.
pj_bli2 proc near
bli2p	struc	;pj_bli2 parameter structure
	bli2_edi dd ?	;what's there from pushad
	bli2_esi dd ?
	bli2_ebp dd ?
	bli2_esp dd ?
	bli2_ebx dd ?
	bli2_edx dd ?
	bli2_ecx dd ?
	bli2_eax dd ?
	bli2_ret dd ?	;return address for function
	bli2_dv	dd ?	;1st parameter - destination screen
	bli2_dx	dd ?
	bli2_dy	dd ?
	bli2_w	dd ?
	bli2_h	dd ?
	bli2_sx	dd ?
	bli2_sy	dd ?
	bli2_sp	dd ?
	bli2_sbpr dd ?
	bli2_color dd ?
	bli2_bcolor dd ?
bli2p ends
	pushad
	mov ebp,esp
	push es
lvarspace equ 16
pushspace equ 4
spt	equ	dword ptr [ebp-pushspace-4]
dpt equ dword ptr [ebp-pushspace-8]
dbpr equ dword ptr [ebp-pushspace-12]
	sub esp,lvarspace	;space for local variables

	mov edi,[ebp].bli2_dv	;get dest screen structure

	;get starting source address in spt
	mov	eax,[ebp].bli2_sy
	mul	[ebp].bli2_sbpr	;y line offset in ax
	mov	ebx,[ebp].bli2_sx
	shr	ebx,3	; += (sx>>3)
	add	eax,ebx	;start source offset in ax
	add eax,[ebp].bli2_sp
	mov spt,eax


	;get starting destination address in es:dpt
	mov eax,[edi].bym_bpr
	mov dbpr,eax		;save a handy copy of dest bpr in local variable
	mul [ebp].bli2_dy
	add eax,[edi].bym_p
	add eax,[ebp].bli2_dx
	mov dpt,eax
	mov ax,[edi].bym_pseg
	mov es,ax


	;calculate start mask for line into dl
	mov ecx,[ebp].bli2_sx
	and ecx,7
	mov dl,80h
	shr	dl,cl	;and devote dl to it...

	mov	eax,[ebp].bli2_color
	mov ebx,[ebp].bli2_bcolor
	jmp	zabline
abline:
	mov	ecx,[ebp].bli2_w	;dot count in ecx
	mov	dh,dl		;get mask into dh
	mov	esi,spt
	mov	edi,dpt
	mov	ah,[esi]		;fetch 1st byte of source into ah
	inc	esi
abpix:
	test	ah,dh
	jnz	abset
	mov es:[edi],bl
	inc	edi	;skip pixel in dest
	shr	dh,1
	jz	newsrc
	loop	abpix
	jmp	zline
abset:	stosb		;set pixel in dest
	shr	dh,1
	jz	newsrc
	loop	abpix
zline:	mov	ecx,[ebp].bli2_sbpr
	add	spt,ecx
	mov	ecx,dbpr
	add	dpt,ecx
zabline:	dec	[ebp].bli2_h
	js	za2
	jmp	abline

newsrc:	;get next byte of source
	mov	ah,[esi]		;fetch byte of source into ah
	inc	esi
	mov	dh,80h		;mask to 1st pixel in byte
	loop	abpix
	jmp	zline

za2:

	add esp,lvarspace	;clear off local variables
	pop es
	popad
	ret

pj_bli2 endp


code	ends
	end

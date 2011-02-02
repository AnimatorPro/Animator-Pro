
	include evga.i



_text	segment	para public use32 'code'
	assume cs:CGROUP,ds:DGROUP

	public _evga_mask1
_evga_mask1 proc near
vmk1p	struc	;vmk1 parameter structure
	vmk1_edi dd ?	;what's there from pushad
	vmk1_esi dd ?
	vmk1_ebp dd ?
	vmk1_esp dd ?
	vmk1_ebx dd ?
	vmk1_edx dd ?
	vmk1_ecx dd ?
	vmk1_eax dd ?
	vmk1_ret dd ?	;return address for function
	vmk1_sp	dd ?
	vmk1_sbpr dd ?
	vmk1_sx	dd ?
	vmk1_sy	dd ?
	vmk1_daddress	dd ?	
	vmk1_bank	dd ?
	vmk1_bpr	dd ?
	vmk1_w	dd ?
	vmk1_h	dd ?
	vmk1_color dd ?
vmk1p ends
	pushad
	mov ebp,esp
	push es
lvarspace equ 16
pushspace equ 4
spt	equ	dword ptr [ebp-pushspace-4]
dpt equ [ebp].vmk1_daddress
	sub esp,lvarspace	;space for local variables

	;get starting source address in spt
	mov	eax,[ebp].vmk1_sy
	mul	[ebp].vmk1_sbpr	;y line offset in ax
	mov	ebx,[ebp].vmk1_sx
	shr	ebx,3	; += (sx1>>3)
	add	eax,ebx	;start source offset in ax
	add eax,[ebp].vmk1_sp
	mov spt,eax

	;Set up correct bank for 1st line (assume all blit in bank)
	mov ecx,[ebp].vmk1_bank
	setbank

	;get starting destination address in es:dpt.  Dest bpr in ebx
	mov ax,PHAR_REAL_SEG
	mov es,ax
	mov ebx,[ebp].vmk1_bpr


	;calculate start mask for line into dl
	mov ecx,[ebp].vmk1_sx
	and ecx,7
	mov dl,80h
	shr	dl,cl	;and devote dl to it...

	mov	eax,[ebp].vmk1_color
abline:
	mov	ecx,[ebp].vmk1_w	;dot count in ecx
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
zline:	mov	ecx,[ebp].vmk1_sbpr
	add	spt,ecx
	add	dpt,ebx
           cmp       dpt, 0B0000H      ; Check if wrapped
           jb        zabline
           sub       dpt, 010000H       ; Bring back to 64K index
           nextbank                     ; Bump to next bank.
zabline:	dec	[ebp].vmk1_h
	jz	za1
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

_evga_mask1 endp

_text	ends
	end


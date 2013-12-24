
	include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


;void JREGS pj_bym_set_rast(Bytemap *v, RCOLOR color);
	public pj_bym_set_rast
pj_bym_set_rast proc near
bsrp struc
	bsr_ebp dd ?
	bsr_ret dd ?
	bsr_v dd ?
	bsr_color dd ?
bsrp ends
	push ebp
	mov ebp,esp
	push ecx
	push esi
	push edi
	push es

	;put screen pixel address into es:di
	mov esi,[ebp].bsr_v
	mov ax,[esi].bym_pseg
	mov es,ax
	mov edi,[esi].bym_p

	;get color and replicate it into all 4 bytes of eax
	mov eax,[ebp].bsr_color
	mov ah,al
	mov cx,ax
	shl eax,16
	mov ax,cx

	;now do the bulk of the color stuffing
	mov ecx,[esi].bym_psize
	shr ecx,2
	rep stosd

	;if any bytes left at end (psize not even multiple of 4) take care of 'em
	mov ecx,[esi].bym_psize
	and ecx,3
	jecxz bsr_000
	rep stosb

bsr_000:
	pop es
	pop edi
	pop esi
	pop ecx
	pop ebp
	ret
pj_bym_set_rast endp

code	ends
	end

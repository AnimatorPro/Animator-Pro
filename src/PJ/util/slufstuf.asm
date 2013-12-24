CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


;	clear_seg(dest, seg, size)
;		clear a (possibly in another segment) bunch of memory.
;		Assume size is divisible by 4.
	public clear_seg
clear_seg proc near
pcs struc
	pcs_bs	dd ?
	pcs_ret dd ?
	pcs_dest dd ?
	pcs_seg dd ?
	pcs_size dd ?
pcs ends
	push ebp
	mov ebp,esp
	push es
	push edi
	push ecx

	mov eax,[ebp].pcs_seg
	mov es,ax
	mov edi,[ebp].pcs_dest
	mov ecx,[ebp].pcs_size
	shr ecx,2
	mov eax,0
	rep stosd

	pop ecx
	pop edi
	pop es
	pop ebp
	ret
clear_seg endp
;copy_seg(soff, sseg, doff, dseg, size)
;  copy lots of data (4 bytes at a time) from source to dest.  Source
;  and dest may be in different segments!
	public copy_seg
copy_seg proc near
pcps struc
	pcps_bs	dd ?
	pcps_ret dd ?
	pcps_soff dd ?
	pcps_sseg dd ?
	pcps_doff dd ?
	pcps_dseg dd ?
	pcps_size dd ?
pcps ends
	push ebp
	mov ebp,esp
	push es
	push ds
	push edi
	push esi
	push ecx

	mov eax,[ebp].pcps_dseg
	mov es,ax
	mov edi,[ebp].pcps_doff
	mov eax,[ebp].pcps_sseg
	mov ds,ax
	mov esi,[ebp].pcps_soff
	mov ecx,[ebp].pcps_size
	shr ecx,2
	rep movsd

	pop ecx
	pop esi
	pop edi
	pop ds
	pop es
	pop ebp
	ret
copy_seg endp

;copy_bseg(soff, sseg, doff, dseg, size)
;  copy lots of data (1 bytes at a time) from source to dest.  Source
;  and dest may be in different segments!
	public copy_bseg
copy_bseg proc near
	push ebp
	mov ebp,esp
	push es
	push ds
	push edi
	push esi
	push ecx

	mov eax,[ebp].pcps_dseg
	mov es,ax
	mov edi,[ebp].pcps_doff
	mov eax,[ebp].pcps_sseg
	mov ds,ax
	mov esi,[ebp].pcps_soff
	mov ecx,[ebp].pcps_size
	rep movsb

	pop ecx
	pop esi
	pop edi
	pop ds
	pop es
	pop ebp
	ret
copy_bseg endp

code	ends
	end

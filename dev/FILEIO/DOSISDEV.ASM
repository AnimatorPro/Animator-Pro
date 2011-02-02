CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i

	public pj_dis_drive
;Boolean pj_dis_drive(int drive) ;
;	drive = 0 for A:, 1 for B: etc.  Does a 'get disk size info' call
;   to see if drive is really there.
pj_dis_drive proc near
jisdp	struc
	jisd_ds dd ?
	jisd_edx dd ?
	jisd_ecx dd ?
	jisd_ebx dd ?
	jisd_ret dd ?
	jisd_drive dd ?
jisdp	ends
	push ebx
	push ecx
	push edx
	push ds

	mov edx,[esp].jisd_drive
	inc edx		;0 = default for this call, 1 = A: ...
	mov ah,1ch
	int 21h
	cmp	al,0ffh
	jz jisd_bad
	mov eax,success
	jmp jisd_end
jisd_bad:
	mov eax,err_nogood
jisd_end:
	pop ds
	pop edx
	pop ecx
	pop ebx
	ret
pj_dis_drive endp

code	ends
	end

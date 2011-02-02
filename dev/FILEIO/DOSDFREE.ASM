CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i

	public pj_ddfree
; long pj_ddfree(int drive);	/* 0 = current.  1 = A:  2 = B: etc. */
pj_ddfree proc near
dfrp	struc
	dfr_edx dd ?
	dfr_ecx dd ?
	dfr_ebx dd ?
	dfr_ret dd ?
	dfr_drive dd ?
dfrp	ends
	push ebx
	push ecx
	push edx

	mov edx,[esp].dfr_drive
	mov ah,36h
	int 21h

	and eax,0ffffh		;mask down anything left in hi bits...
	and ebx,0ffffh
	and ecx,0ffffh
	mul ebx			;multiply sectors/cluster times fre clusters
	mul ecx			;and multiply result by bytes/sector

	pop edx
	pop ecx 
	pop ebx
	ret
pj_ddfree endp


code	ends
	end

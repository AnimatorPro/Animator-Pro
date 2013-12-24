CGROUP	group	code
; This code works better for bernoullis but worse for CD roms than the
; shipping code in dosisdev.asm.

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i

	public pj_dis_drive
;Boolean pj_dis_drive(int drive) ;
;	drive = 0 for A:, 1 for B: etc.  Does call to see if it's a floppy
;   actually,  but only uses error status.
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

	mov ebx,[esp].jisd_drive
	inc ebx		;0 = default for this call, 1 = A: ...
	mov ah,44h
	mov al,08h
	int 21h
	jc jisd_bad
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

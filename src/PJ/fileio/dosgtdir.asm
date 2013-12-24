	EXTRN pj_dget_err:word

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i

	public pj_dget_dir
; Errcode pj_dget_dir(int drive, char *dir);	/* 0 = current, 1 = A: for drive */
; returns 0 if ok < 0 if error
pj_dget_dir proc near
jgdi	struc
	jgdi_esi dd ?
	jgdi_edx dd ?
	jgdi_ret dd ?
	jgdi_drive dd ?
	jgdi_dir dd ?
jgdi	ends
	push edx
	push esi

	mov edx,[esp].jgdi_drive
	mov esi,[esp].jgdi_dir
	mov ah,47h
	int 21h
	jc jgdi_bad
	mov eax,success
	jmp jgdi_end
jgdi_bad:
	call near ptr pj_dget_err
jgdi_end:
	pop esi
	pop edx
	ret
pj_dget_dir endp


code	ends
	end

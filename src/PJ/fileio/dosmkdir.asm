CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i

	public pj_dmake_dir
; Boolean pj_dmake_dir(char *path);
pj_dmake_dir proc near
jmdp	struc
	jmd_edx	dd ?
	jmd_ret dd ?
	jmd_path dd ?
jmdp	ends
	push edx
	mov edx,[esp].jmd_path
	mov ah,39h
	int 21h
	jc jmd_bad
	mov eax,success
	jmp jmd_end
jmd_bad:
	mov eax,err_nogood
jmd_end:
	pop edx
	ret
pj_dmake_dir endp


code	ends
	end

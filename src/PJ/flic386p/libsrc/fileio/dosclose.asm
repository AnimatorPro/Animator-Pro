CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i

;	void pj_dclose(int file_handle)
	public pj_dclose
pj_dclose proc near
pdos_close struc
	savebx dd ?
	pdos_closeret	dd ?
	handle dd ?
pdos_close ends
	push ebx
	mov ah,3eh
	mov ebx,[esp].handle
	int 21h
	pop ebx
	ret
pj_dclose endp

code	ends
	end

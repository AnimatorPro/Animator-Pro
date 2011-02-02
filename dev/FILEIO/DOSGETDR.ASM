CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i


	public pj_dget_drive
;int pj_dget_drive(void);	/* returns current drive.  0 = A: 1 = B: ... */
pj_dget_drive proc near
	mov ah,19h
	int 21h
	and eax,0ffh
	ret
pj_dget_drive endp

code	ends
	end

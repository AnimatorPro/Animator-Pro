	EXTRN pj_dget_err:word

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i

;	pj_dtruncate(int file_handle)
;	truncates file at current offset, returns dos error code if not ok
	public pj_dtruncate
pj_dtruncate proc near
pjtruncate struc
	pjt_savebp dd ?
	pjt_ret dd ?
	pjt_handle dd ?
pjtruncate ends
	push ebp
	mov ebp,esp
	push ebx
	push ecx
	push edx
	mov ebx,[ebp].pjt_handle
	xor ecx,ecx  ; 0 in ecx
	mov edx,ebp  ; any buffer, I'll use stack
	mov ah,40h
	int 21h
	jnc #retok
	mov dword ptr [edx],0
	call near ptr pj_dget_err
#retok:
	pop edx
	pop ecx
	pop ebx
	pop ebp
	ret
pj_dtruncate endp

code	ends
	end

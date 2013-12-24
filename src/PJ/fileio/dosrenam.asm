	EXTRN pj_dget_err:word

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i

	;pj_drename(char *oldname, char *newname);
	; renames the file.  Will not do it if newname already exists.
	public pj_drename
pj_drename proc near
pjrn	struc
	prjn_ebs dd ?
	prjn_ret dd ?
	prjn_old dd ?
	prjn_new dd ?
pjrn	ends
	push ebp
	mov ebp,esp
	push edx
	push edi
	mov	edx,[ebp].prjn_old
	mov edi,[ebp].prjn_new
	mov ah,56h
	int 21h
	jnc #retgood
	call near ptr pj_dget_err
	jmp	#retit
#retgood:
	mov eax,0
#retit:
	pop edi
	pop edx
	pop ebp
	ret
pj_drename endp

code	ends
	end

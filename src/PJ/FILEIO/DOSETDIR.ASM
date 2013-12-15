	EXTRN pj_dget_err:word

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i

; I renamed it cause it is so picky -- use change_dir() in dosstuff.c  Peter

	public _lodos_set_dir ; ** does not handle a trailing '\' **
; Errcode _lodos_set_dir(char *path);
_lodos_set_dir proc near
jsdip	struc
	jsdi_edx dd ?
	jsdi_ret dd ?
	jsdi_path dd ?
jsdip	ends
	push edx
	mov edx,[esp].jsdi_path
	mov ah,3bh
	int 21h
	jc jsdi_bad
	mov eax,success
	jmp jsdi_end
jsdi_bad:
	call near ptr pj_dget_err
jsdi_end:
	pop edx
	ret
_lodos_set_dir endp


code	ends
	end

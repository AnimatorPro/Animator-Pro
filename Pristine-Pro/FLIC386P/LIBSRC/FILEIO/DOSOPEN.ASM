	EXTRN pj_dget_err:word

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

;	Doserr pj_dopen(int *phandle, char *name, int mode)
;			mode = 0 read only
;			mode = 1 write only
;			mode = 2 read/write
;
;		Returns Dsuccess (0) on success the dos error code on failure
;		if successful *phandle will have a valid file handle placed in it.
; 		mode is ignored in this call

	public pj_dopen
pj_dopen proc near
pdos_open struc
	savedx dd ?
	pdos_openret	dd ?
	phandle dd ?
	filename dd ?
	mode dd ?
pdos_open ends
	push edx
	mov edx,[esp].filename
	mov eax,[esp].mode
	mov ah,3dh
	int 21h
	mov edx,[esp].phandle
	jnc #goodret
	mov dword ptr [edx],0
	call near ptr pj_dget_err
	jmp #out
#goodret:
	mov dword ptr [edx],eax
	mov eax,0
#out:
	pop edx
	ret
pj_dopen endp

code	ends
	end

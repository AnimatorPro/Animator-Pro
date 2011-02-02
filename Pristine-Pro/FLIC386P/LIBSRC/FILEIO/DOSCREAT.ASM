	EXTRN pj_dget_err:word


CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i

;	Doserr pj_dcreate(int *phandle, name, int fmode)
;
;		Returns Dsuccess (0) on success the dos error code on failure
;		if successful *phandle will have a valid file handle placed in it.
; 		mode is ignored in this call

	public pj_dcreate
pj_dcreate proc near
pdos_create struc
	pjc_savedx dd ?
	pjc_savecx dd ?
	pjc_ret	dd ?
	pjc_phandle dd ?
	pjc_name dd ?
	pjc_fmode dd ?
pdos_create ends
	push ecx
	push edx
	mov ecx,0	;attributes always zero for normal file...
	mov edx,[esp].pjc_name
	mov ah,3ch
	int 21h
	mov edx,[esp].pjc_phandle
	jnc #pmscret
	mov dword ptr [edx],0
	call near ptr pj_dget_err
	jmp #out

#pmscret:
	mov dword ptr [edx],eax
	mov eax,0
#out:
	pop edx
	pop ecx
	ret
pj_dcreate endp

code	ends
	end

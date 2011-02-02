	EXTRN pj_dget_err:word

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i


	public lo_dos_set_drive
;int lo_dos_set_drive(int drive); /* 0 = A: 1 = B: .. */
; returns 0 if ok < 0 if error */
; Unfortunately (due to bug in Phar Lap??) if drive is A: or B:,
; this will return an error when it really shouldn't.  Work around
; is a layer on top which checks to make sure we made it to the
; device in this case.
lo_dos_set_drive proc near
jsdp	struc
	jsd_edx	dd ?
	jsd_ret	dd ?
	jsd_drive 	dd ?
jsdp	ends
	push edx
	mov	edx,[esp].jsd_drive
	mov ah,0eh
	int 21h
	jc  jsdrbad
	mov eax,success
	jmp jsdrend
jsdrbad:
	call near ptr pj_dget_err
jsdrend:
	pop edx
	ret
lo_dos_set_drive endp

code	ends
	end

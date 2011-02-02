CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i


;	pj_ddelete(name)
; returns errcode value.  
	public pj_ddelete
pj_ddelete proc near
pjdelete struc
	pjd_savedx dd ?
	pjd_ret	dd ?
	pjd_name dd ?
pjdelete ends
	push edx
	mov edx,[esp].pjd_name
	mov ah,41h
	int 21h
	jnc #retgood
	mov eax,err_nogood
	jmp	#retit
#retgood:
	mov eax,success
#retit:
	pop edx
	ret
pj_ddelete endp

code	ends
	end

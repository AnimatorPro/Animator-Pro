CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i


	public pj_dfirst
;	Boolean pj_dfirst(char *pattern, int attribute) ;
pj_dfirst proc near
jsfp	struc
	jsf_edx	dd ?
	jsf_ecx dd ?
	jsf_ret dd ?
	jsf_pattern dd ?
	jsf_attribute dd ?
jsfp	ends
	push ecx
	push edx
	mov	edx,[esp].jsf_pattern
	mov ecx,[esp].jsf_attribute
	mov ah,4eh
	int 21h
	jc  jsfbad
	mov eax,1
	jmp	jsfret
jsfbad:
	xor eax,eax	;zero out result
jsfret:
	pop ecx
	pop edx
	ret
pj_dfirst endp

code	ends
	end

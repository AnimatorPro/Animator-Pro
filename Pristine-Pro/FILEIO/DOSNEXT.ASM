CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i


	public pj_dnext
;	Boolean pj_dnext(void);
pj_dnext proc near
	mov ah,4fh
	int 21h
	jc jsnbad
	mov eax,1
	jmp jsnret
jsnbad:
	xor eax,eax ;zero out result
jsnret:
	ret
pj_dnext endp

code	ends
	end

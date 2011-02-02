
;pointer.asm - A couple of routines mostly to fool the C compiler
;	and generally help in converting between integer and pointer
;	types
;
;


CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

;void *long_to_pt(long l);
	public long_to_pt
long_to_pt proc near
	mov eax,4[esp]
	ret
long_to_pt endp

;long pt_to_long(void *p);
	public pt_to_long
pt_to_long proc near
	mov eax,4[esp]
	ret
pt_to_long endp

code	ends
	end

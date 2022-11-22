CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i

	public dos_key_shift
;dos_key_shift() 
;	returns keyboard shift/control/alt state
dos_key_shift proc near
	mov ah,2
	int 16h
	and eax,0ffh
	ret
dos_key_shift endp

code	ends
	end


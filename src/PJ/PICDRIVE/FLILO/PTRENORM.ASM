
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


	public pj_enorm_pointer
;	pj_enorm_pointer(ptr)
;		forces a pointer to even allignment on 386.  On 8086 would also
;		fold in as much as possible of the offset into the segment.
pj_enorm_pointer proc near
	mov	eax,[esp+4]
	inc eax
	and eax,0fffffffeh
	ret
pj_enorm_pointer endp

code	ends
	end


CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i

	public pj_dcount_floppies
; int pj_dcount_floppies(void);
;	ask bios how many floppies are installed
pj_dcount_floppies proc near
	push ebx	;I'm afraid I'm not sure what register this one effects
	push ecx	; so to be safe, push the world....
	push edx
	push esi
	push edi
	push ds
	push es

	int 11h
	test al,1	;if lo bit of a register zero, no information here
	jz	jcfnone
	shr	eax,6	;else extract the floppy count bits
	and eax,3
	inc eax
	jmp jcfend
jcfnone:
	sub eax,eax

jcfend:
	pop es
	pop ds
	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx
	ret
pj_dcount_floppies endp

code	ends
	end

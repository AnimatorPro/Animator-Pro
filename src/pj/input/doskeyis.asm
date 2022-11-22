CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i
include useful.i


;pj_key_is() - returns 1 if a key is ready to read from keyboard, 0 otherwise
	public pj_key_is
pj_key_is proc near
	mov edx,041Ah
	mov dx,gs:[edx]
	mov eax,041Ch
	mov ax,gs:[eax]
	cmp ax,dx		;Are they the same?
	jz #ret0
	mov eax,1
	jmp #retit
#ret0:
	xor eax,eax
#retit:
	ret
pj_key_is endp

code	ends
	end

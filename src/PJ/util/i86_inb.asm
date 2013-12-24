
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


;	i86_inb(int port)
	public i86_inb
i86_inb proc near
inb_p struc
	inb_savedx dd ?
	inb_ret	dd ?
	inb_port dd ?
inb_p ends
	push edx
	mov edx,[esp].inb_port
	xor eax,eax
	in al,dx
	pop edx
	ret
i86_inb endp


code	ends
	end

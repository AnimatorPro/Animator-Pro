
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


;	i86_inw(int port)
	public i86_inw
i86_inw proc near
inw_p struc
	inw_savedx dd ?
	inw_ret	dd ?
	inw_port dd ?
inw_p ends
	push edx
	mov edx,[esp].inw_port
	xor eax,eax
	in ax,dx
	pop edx
	ret
i86_inw endp


code	ends
	end


CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

;	i86_outw(int port, int value)
	public i86_outw
i86_outw proc near
otw_p struc
	otw_savedx dd ?
	otw_ret	dd ?
	otw_port dd ?
	otw_value dd ?
otw_p ends
	push edx
	mov edx,[esp].otw_port
	mov eax,[esp].otw_value
	out dx,ax
	pop edx
	ret
i86_outw endp


code	ends
	end
